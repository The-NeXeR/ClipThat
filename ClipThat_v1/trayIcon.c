#include "trayIcon.h"
#include <wchar.h>

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

void trayIconDestroy(void) {
	if (!iconAdded) return;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	iconAdded = false;
}

void trayIconShowContextMenu(HWND ownerHwnd) {
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL) { return; }
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_HISTORY, L"Show History");
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow(ownerHwnd);
	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, ownerHwnd, NULL);
	PostMessage(ownerHwnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
}