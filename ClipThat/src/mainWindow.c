#include "mainWindow.h"
#include "trayIcon.h"
#include "historyWindow.h"
#include "clipboard.h"
#include "history.h"
#include "session.h"

#define HOTKEY_SHOW_HISTORY 1
#define MAX_HISTORY_COUNT 50
#define MAIN_WINDOW_CLASS_NAME L"MyWindow"

static ClipboardEntry* head = NULL;
static HWND lastForegroundWindow = NULL;

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_QUERYENDSESSION:
		return TRUE;

	case WM_ENDSESSION:
		if (wParam) {
			if (!sessionSave(head)) {
				OutputDebugStringW(L"MainWndProc: failed to save clipboard history on session end.\n");
			}
		}
		return 0;

	case WM_DESTROY:
		if (!sessionSave(head)) {
			OutputDebugStringW(L"MainWndProc: failed to save clipboard history on exit.\n");
		}
		sessionShutdown();
		historyClear(&head);
		RemoveClipboardFormatListener(hwnd);
		UnregisterHotKey(hwnd, HOTKEY_SHOW_HISTORY);
		trayIconDestroy();
		PostQuitMessage(0);
		return 0;

	case WM_TRAYICON:
		if (lParam == WM_RBUTTONUP) {
			lastForegroundWindow = GetForegroundWindow();
			trayIconShowContextMenu(hwnd);
		}
		return 0;

	case WM_HOTKEY:
		if (wParam == HOTKEY_SHOW_HISTORY) {
			lastForegroundWindow = GetForegroundWindow();
			historyWindowShow(head, lastForegroundWindow);
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TRAY_SHOW_HISTORY:
			historyWindowShow(head, lastForegroundWindow);
			break;
		case ID_TRAY_EXIT:
			DestroyWindow(hwnd);
			break;
		}
		return 0;

	case WM_CLIPBOARDUPDATE:
		if (clipboardHandleUpdate(hwnd, &head, MAX_HISTORY_COUNT)) {
			historyWindowRefresh(head);
		}
		return 0;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

bool mainWindowCreate(HINSTANCE hInstance) {
	if (!sessionInit(&head)) {
		MessageBoxW(
			NULL,
			L"Failed to initialize the history database. Your clipboard history will not be saved this session.",
			L"ClipThat - Database Error",
			MB_ICONWARNING
		);
	}
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	if (!RegisterClassEx(&wc)) {
		MessageBoxW(NULL, L"Failed to register the main window class.", L"ClipThat - Startup Error", MB_ICONERROR);
		return false;
	}
	HWND hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		MAIN_WINDOW_CLASS_NAME,
		L"ClipThat",
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
		NULL, NULL, hInstance, NULL
	);
	if (hwnd == NULL) {
		MessageBoxW(NULL, L"Failed to create the main window.", L"ClipThat - Startup Error", MB_ICONERROR);
		return false;
	}
	if (!trayIconCreate(hwnd)) {
		MessageBoxW(NULL, L"Failed to create tray icon.", L"ClipThat - Startup Error", MB_ICONERROR);
		DestroyWindow(hwnd);
		return false;
	}
	if (!RegisterHotKey(hwnd, HOTKEY_SHOW_HISTORY, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'H')) {
		MessageBoxW(NULL, L"Failed to register hotkey.", L"ClipThat - Startup Warning", MB_ICONWARNING);
	}
	if (!AddClipboardFormatListener(hwnd)) {
		MessageBoxW(NULL, L"Failed to register clipboard listener.", L"ClipThat - Startup Error", MB_ICONERROR);
		DestroyWindow(hwnd);
		return false;
	}
	return true;
}