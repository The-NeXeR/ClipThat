#include "trayIcon.h"
#include <wchar.h>
#include <stdlib.h> // _TRUNCATE

static NOTIFYICONDATA nid = { 0 };
static bool iconAdded = false;

bool trayIconCreate(HWND ownerHWND) {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = ownerHWND;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(wchar_t), L"ClipThat");

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) { return false; }
	iconAdded = true;
	return true;
}

bool trayIconNotify(const wchar_t* title, const wchar_t* message) {
	if (!iconAdded) return false;
	nid.uFlags = NIF_INFO;
	// _TRUNCATE: a too-long message must degrade to a truncated hint, not abort the app
	wcscpy_s(nid.szInfoTitle, _TRUNCATE, title);
	wcscpy_s(nid.szInfo, _TRUNCATE, message);
	nid.dwInfoFlags = NIIF_INFO | NIIF_RESPECT_QUIET_TIME;

	bool success = Shell_NotifyIconW(NIM_MODIFY, &nid);

	// Restore the normal tray icon configuration.
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	return success;
}

void trayIconDestroy(void) {
	if (!iconAdded) return;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	iconAdded = false;
}

UINT trayIconTaskbarCreatedMessage(void) {
	return RegisterWindowMessageW(L"TaskbarCreated");
}

void trayIconRestore(void) {
	if (nid.hWnd == NULL) { return; }
	if (Shell_NotifyIcon(NIM_ADD, &nid)) { iconAdded = true; }
}

void trayIconShowContextMenu(HWND ownerHwnd) {
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL) { return; }
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_HISTORY, L"Show History");
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_FLUSH_HISTORY, L"Delete History");
	AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow(ownerHwnd);
	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, ownerHwnd, NULL);
	PostMessage(ownerHwnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
}