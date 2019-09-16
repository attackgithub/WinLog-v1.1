#include "main.h"

HINSTANCE hInstance = NULL;

LRESULT CALLBACK KeyboardMsgProc(int code , WPARAM wParam, LPARAM lParam);

HHOOK hMsgHook = NULL;
UINT KBoardMessage = NULL;
HWND main_window = NULL;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
        hInstance = (HINSTANCE)hModule;

    return TRUE;
}

int HOOKDLL set_hook(HWND hWnd, UINT UpdateMsg)
{
    if(hWnd == NULL)
        return -1;

    main_window = hWnd;
    KBoardMessage = UpdateMsg;

    hMsgHook = SetWindowsHookEx(WH_GETMESSAGE, KeyboardMsgProc, hInstance, 0);
    if(hMsgHook == NULL)
        return -1;

    return 0;
}

int HOOKDLL unset_hook()
{
    UnhookWindowsHookEx(hMsgHook);
    hMsgHook = NULL;

    return 0;
}

LRESULT CALLBACK KeyboardMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
    if(code > 0)
    {
        MSG *msg = (MSG *)lParam;

        if((lParam) && (msg->message == WM_CHAR) && (wParam == WM_COMMAND))
            PostMessage(main_window, KBoardMessage, msg->wParam, 0);
    }

    return CallNextHookEx(hMsgHook, code, wParam, lParam);
}
