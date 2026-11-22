#include "trayIcon.h"
#include "resources.h"
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
	nid.hIcon = (HICON)LoadImageW(
		GetModuleHandleW(NULL),
		MAKEINTRESOURCEW(IDI_ICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR
	);
	wcsncpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]), L"ClipThat", _TRUNCATE);

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) { return false; }
	iconAdded = true;
	return true;
}

bool trayIconNotify(const wchar_t* title, const wchar_t* message) {
	if (!iconAdded) return false;
	nid.uFlags = NIF_INFO;
	wcsncpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(nid.szInfoTitle[0]), title, _TRUNCATE);
	wcsncpy_s(nid.szInfo, sizeof(nid.szInfo) / sizeof(nid.szInfo[0]), message, _TRUNCATE);
	nid.dwInfoFlags = NIIF_INFO | NIIF_RESPECT_QUIET_TIME;
	bool success = Shell_NotifyIconW(NIM_MODIFY, &nid);

	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	return success;
}
bool trayIconSetTip(const wchar_t* tip) {
	if (!iconAdded) { return false; }
	wcsncpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]), tip, _TRUNCATE);
	return Shell_NotifyIcon(NIM_MODIFY, &nid) != FALSE;
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

void trayIconShowContextMenu(HWND ownerHwnd, bool paused) {
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL) { return; }
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW_HISTORY, L"Show History");
	AppendMenuW(hMenu, MF_STRING, ID_TRAY_PAUSE_CAPTURE, paused ? L"Resume Capture" : L"Pause Capture");
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