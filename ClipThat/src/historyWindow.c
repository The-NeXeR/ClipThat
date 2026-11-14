#include "historyWindow.h"
#include "historyView.h"
#include "history.h"
#include "paste.h"
#include "clipboard.h"
#include "session.h"
#include "config.h"
#include "settings.h"
#include "resources.h"
#include <commctrl.h>

#define LISTBOX_ID 2000
#define SEARCHBOX_ID 2001
#define CLASS_NAME L"HistoryWindow"

static HWND historyHwnd = NULL;
static HWND listBoxHwnd = NULL;
static HWND searchBoxHwnd = NULL;
static HWND pasteTargetWindow = NULL;
static const ClipboardEntry* historyHead = NULL;
static wchar_t searchQuery[SEARCH_QUERY_MAX] = { 0 };
static ClipboardEntry* searchResults = NULL;

static void refreshHistoryList(void) {
	if (listBoxHwnd == NULL) { return; }
	SendMessage(listBoxHwnd, LB_RESETCONTENT, 0, 0);
	historyClear(&searchResults);
	if (searchQuery[0] == L'\0') {
		historyPopulateListBox(listBoxHwnd, historyHead, NULL);
		return;
	}
	historyPopulateListBox(listBoxHwnd, historyHead, searchQuery);
	int shownCount = (int)SendMessage(listBoxHwnd, LB_GETCOUNT, 0, 0);
	int remaining = settingsGet()->searchResultLimit - shownCount;
	if (remaining > 0 && sessionSearch(searchQuery, remaining, &searchResults)) {
		historyAppendSearchResults(listBoxHwnd, searchResults, searchQuery, historyHead);
	}
	else {
		historyClear(&searchResults);
	}
}

static void historyWindowPasteSelection(void) {
	int index = (int)SendMessage(listBoxHwnd, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR) { return; }
	LRESULT itemData = SendMessage(listBoxHwnd, LB_GETITEMDATA, index, 0);
	if (itemData == LB_ERR || itemData == 0) { return; }
	const ClipboardEntry* entry = (const ClipboardEntry*)itemData;
	if (!clipboardSetText(historyHwnd, entry->text)) {
		OutputDebugStringW(L"HistoryWndProc: failed to copy history entry to clipboard.\n");
		return;
	}
	ShowWindow(historyHwnd, SW_HIDE);
	pasteSimulate(pasteTargetWindow);
}

static LRESULT CALLBACK SearchBoxSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclassId, DWORD_PTR refData) {
	switch (message) {
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (searchQuery[0] != L'\0') { SetWindowTextW(hwnd, L""); }
			else { historyWindowHide(); }
			return 0;
		}
		if (wParam == VK_RETURN) {
			historyWindowPasteSelection();
			return 0;
		}
		if (wParam == VK_DOWN) {
			int count = (int)SendMessage(listBoxHwnd, LB_GETCOUNT, 0, 0);
			if (count > 0) {
				SendMessage(listBoxHwnd, LB_SETCURSEL, 0, 0);
				SetFocus(listBoxHwnd);
			}
			return 0;
		}
		break;
	case WM_CHAR:
		if (wParam == VK_RETURN || wParam == VK_ESCAPE) { return 0; }
		break;
	}
	(void)subclassId;
	(void)refData;
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK ListBoxSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR subclassId, DWORD_PTR refData) {
	switch (message) {
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			historyWindowPasteSelection();
			return 0;
		}
		if (wParam == VK_ESCAPE) {
			historyWindowHide();
			return 0;
		}
		break;
	}
	(void)subclassId;
	(void)refData;
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK HistoryWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE:
	{
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == SEARCHBOX_ID && HIWORD(wParam) == EN_CHANGE) {
			int length = GetWindowTextW(searchBoxHwnd, searchQuery, SEARCH_QUERY_MAX);
			searchQuery[length] = L'\0';
			refreshHistoryList();
			return 0;
		}
		if (LOWORD(wParam) == LISTBOX_ID && HIWORD(wParam) == LBN_DBLCLK) {
			historyWindowPasteSelection();
		}
		return 0;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		MoveWindow(searchBoxHwnd, 10, 10, width - 20, 28, TRUE);
		MoveWindow(listBoxHwnd, 10, 46, width - 20, height - 56, TRUE);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

bool historyWindowCreate(HINSTANCE hInstance) {
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = HistoryWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
	wc.hIconSm = wc.hIcon;

	if (!RegisterClassEx(&wc)) {
		OutputDebugStringW(L"historyWindowCreate: failed to register window class.\n");
		return false;
	}

	historyHwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Clipboard History",
		WS_OVERLAPPEDWINDOW, // no WS_VISIBLE -- starts hidden by design
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
		NULL, NULL, hInstance, NULL
	);
	if (historyHwnd == NULL) {
		OutputDebugStringW(L"historyWindowCreate: failed to create history window.\n");
		return false;
	}

	searchBoxHwnd = CreateWindowExW(
		0,
		L"EDIT",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		10, 10, 460, 28,
		historyHwnd,
		(HMENU)SEARCHBOX_ID,
		hInstance,
		NULL
	);
	if (searchBoxHwnd == NULL) {
		OutputDebugStringW(L"historyWindowCreate: failed to create search box.\n");
		DestroyWindow(historyHwnd);
		historyHwnd = NULL;
		return false;
	}
	SetWindowSubclass(searchBoxHwnd, SearchBoxSubclassProc, 1, 0);

	listBoxHwnd = CreateWindowEx(
		0,
		L"LISTBOX",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		10, 46, 460, 292,
		historyHwnd,
		(HMENU)LISTBOX_ID,
		hInstance,
		NULL
	);
	if (listBoxHwnd == NULL) {
		OutputDebugStringW(L"historyWindowCreate: failed to create listbox.\n");
		DestroyWindow(historyHwnd);
		historyHwnd = NULL;
		return false;
	}
	SetWindowSubclass(listBoxHwnd, ListBoxSubclassProc, 1, 0);

	return true;
}

void historyWindowRefresh(const ClipboardEntry* head) {
	historyHead = head;
	refreshHistoryList();
}

void historyWindowHide(void) {
	if (historyHwnd != NULL) { ShowWindow(historyHwnd, SW_HIDE); }
}

bool historyWindowIsVisible(void) {
	return historyHwnd != NULL && IsWindowVisible(historyHwnd);
}

void historyWindowShow(const ClipboardEntry* head, HWND targetWindow) {
	pasteTargetWindow = targetWindow;
	historyHead = head;
	searchQuery[0] = L'\0';
	SetWindowTextW(searchBoxHwnd, L"");
	refreshHistoryList();
	ShowWindow(historyHwnd, SW_SHOW);
	SetForegroundWindow(historyHwnd);
	SetFocus(searchBoxHwnd);
}
