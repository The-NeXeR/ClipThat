#include "historyWindow.h"
#include "historyView.h"
#include "paste.h"
#include "clipboard.h"

#define LISTBOX_ID 2000
#define CLASS_NAME L"HistoryWindow"

static HWND historyHwnd = NULL;
static HWND listBoxHwnd = NULL;
static HWND pasteTargetWindow = NULL;

static LRESULT CALLBACK HistoryWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE:
	{
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == LISTBOX_ID && HIWORD(wParam) == LBN_DBLCLK) {
			int index = (int)SendMessage(listBoxHwnd, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				LRESULT itemData = SendMessage(listBoxHwnd, LB_GETITEMDATA, index, 0);
				if (itemData != LB_ERR) {
					const ClipboardEntry* entry = (const ClipboardEntry*)itemData;
					if (!clipboardSetText(hwnd, entry->text)) {
						OutputDebugStringW(L"HistoryWndProc: failed to copy history entry to clipboard.\n");
						return 0;
					}
					ShowWindow(hwnd, SW_HIDE);
					pasteSimulate(pasteTargetWindow);
				}
			}
		}
		return 0;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		MoveWindow(listBoxHwnd, 10, 10, width - 20, height - 20, TRUE);
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

	listBoxHwnd = CreateWindowEx(
		0,
		L"LISTBOX",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		10, 10, 460, 330,
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

	return true;
}
void historyWindowRefresh(const ClipboardEntry* head) {
	historyPopulateListBox(listBoxHwnd, head);
}

void historyWindowShow(const ClipboardEntry* head, HWND targetWindow) {
	pasteTargetWindow = targetWindow;
	historyWindowRefresh(head);
	ShowWindow(historyHwnd, SW_SHOW);
	SetForegroundWindow(historyHwnd);
}