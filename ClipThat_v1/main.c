#include <windows.h>
#include "mainWindow.h"
#include "historyWindow.h"

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
    if (!mainWindowCreate(hInstance)) { return 1; }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}