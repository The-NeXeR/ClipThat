#include <windows.h>
#include "mainWindow.h"
#include "historyWindow.h"
#include "startup.h"
#include "trayIcon.h"
#include "singleInstanceguard.h"
#include "settings.h"
#include <stdio.h>


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    if (singleInstanceGuard()) {
        HWND existingW = FindWindowW(MAIN_WINDOW_CLASS_NAME, NULL);
        if (existingW != NULL) {
            PostMessageW(existingW, WM_CLIPTHAT_SHOW, 0, 0);
        }
        return 0;
    }
    bool firstRun = !settingsLoad();
    if (!historyWindowCreate(hInstance)) {
        MessageBoxW(NULL, L"Failed to create the history window.", L"ClipThat - Startup Error", MB_ICONERROR);
        return 1;
    }
    if (!mainWindowCreate(hInstance)) { return 1; }
    if (settingsGet()->startWithWindows) {
        if (!startupEnable()) {
            OutputDebugStringW(L"WinMain: failed to register ClipThat for Windows startup.\n");
        }
    }
    else if (!startupDisable()) {
        OutputDebugStringW(L"WinMain: failed to unregister ClipThat from Windows startup.\n");
    }
    if (firstRun) {
        wchar_t hotkeyText[64];
        settingsHotkeyText(hotkeyText, 64);
        wchar_t hint[256];
        swprintf_s(hint, 256, L"ClipThat is running in your system tray. Press %s to open your clipboard history.", hotkeyText);
        if (!trayIconNotify(L"ClipThat", hint)) {
            OutputDebugStringW(L"WinMain: failed to show the first-run tray notification.\n");
        }
    }
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    singleInstanceRelease();
    return (int)msg.wParam;
}
