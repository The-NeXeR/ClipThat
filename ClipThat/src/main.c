#include <windows.h>
#include "mainWindow.h"
#include "historyWindow.h"
#include "startup.h"
#include "trayIcon.h"


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
    if (!historyWindowCreate(hInstance)) {
        MessageBoxW(NULL, L"Failed to create the history window.", L"ClipThat - Startup Error", MB_ICONERROR);
        return 1;
    }
    // Check BEFORE startupEnable() writes the Run key: an absent entry means
    // this is the first launch, which is the only time the hint should show.
    bool firstRun = !startupIsEnabled();
    if (!mainWindowCreate(hInstance)) { return 1; }
    if (!startupEnable()) {
        OutputDebugStringW(L"WinMain: failed to register ClipThat for Windows startup.\n");
    }
    if (firstRun) {
        if (!trayIconNotify(
            L"ClipThat",
            L"ClipThat is running in your system tray. Press Ctrl+Alt+H to open your clipboard history."
        )) {
            OutputDebugStringW(L"WinMain: failed to show the first-run tray notification.\n");
        }
    }
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
