#include "mainWindow.h"
#include "trayIcon.h"
#include "historyWindow.h"
#include "clipboard.h"
#include "history.h"
#include "session.h"
#include "resources.h"
#include "settings.h"
#include <stdio.h>

#define HOTKEY_SHOW_HISTORY 1

static ClipboardEntry* head = NULL;
static HWND lastForegroundWindow = NULL;
static UINT taskbarCreatedMsg = 0;

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_QUERYENDSESSION:
		return TRUE;

	case WM_ENDSESSION:
		return 0;

	case WM_DESTROY:
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
			if (historyWindowIsVisible()) {
				historyWindowHide();
			}
			else {
				lastForegroundWindow = GetForegroundWindow();
				historyWindowShow(head, lastForegroundWindow);
			}
		}
		return 0;
	case WM_CLIPTHAT_SHOW:
		lastForegroundWindow = GetForegroundWindow();
		historyWindowShow(head, lastForegroundWindow);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TRAY_FLUSH_HISTORY:
			if (MessageBoxW(hwnd, L"Are you sure? This will delete all Clipboard's History.", L"Confirmation", MB_YESNO | MB_ICONWARNING) == IDYES) {
				sessionFlush();
				historyClear(&head);
				historyWindowRefresh(head);
			}
			break;
		case ID_TRAY_SHOW_HISTORY:
			historyWindowShow(head, lastForegroundWindow);
			break;
		case ID_TRAY_EXIT:
			DestroyWindow(hwnd);
			break;
		}
		return 0;

	case WM_CLIPBOARDUPDATE:
	{
		ClipboardEntry* previousHead = head;
		if (clipboardHandleUpdate(hwnd, &head, settingsGet()->ramHistoryCount) && head != previousHead) {
			sessionTouch(head->text, settingsGet()->dbHistoryCount);
			historyWindowRefresh(head);
		}
		return 0;
	}

	default:
		if (taskbarCreatedMsg != 0 && message == taskbarCreatedMsg) {
			trayIconRestore();
			return 0;
		}
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
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
	wc.hIconSm = wc.hIcon;

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
	taskbarCreatedMsg = trayIconTaskbarCreatedMessage();
	if (!trayIconCreate(hwnd)) {
		MessageBoxW(NULL, L"Failed to create tray icon.", L"ClipThat - Startup Error", MB_ICONERROR);
		DestroyWindow(hwnd);
		return false;
	}
	if (!RegisterHotKey(hwnd, HOTKEY_SHOW_HISTORY, settingsGet()->hotkeyModifiers | MOD_NOREPEAT, settingsGet()->hotkeyKey)) {
		wchar_t hotkeyText[64];
		settingsHotkeyText(hotkeyText, 64);
		wchar_t warning[256];
		swprintf_s(warning, 256, L"Hotkey %s is in use or reserved. ClipThat still works - use the tray icon, or edit settings.ini to change it.", hotkeyText);
		trayIconNotify(L"ClipThat", warning);
	}
	if (!AddClipboardFormatListener(hwnd)) {
		MessageBoxW(NULL, L"Failed to register clipboard listener.", L"ClipThat - Startup Error", MB_ICONERROR);
		DestroyWindow(hwnd);
		return false;
	}
	return true;
}
