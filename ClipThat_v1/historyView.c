#include "historyView.h"
#include <stdlib.h>
#include <wchar.h>
#define MAX_PREVIEW_LENGTH 120


wchar_t* historyPreviewBuild(const wchar_t* text) {
	if (text == NULL) { return NULL; }
	// I guess its fixed since is not O(length) anymore and its O(min(length, cap) but ill try to optimize it more
	size_t limitation = MAX_PREVIEW_LENGTH + 1;
	size_t length = 0;
	while (length < limitation && text[length] != L'\0') {
		length++;
	}
	bool truncated = (length > MAX_PREVIEW_LENGTH);
	size_t copyLength = truncated ? (MAX_PREVIEW_LENGTH > 3 ? MAX_PREVIEW_LENGTH - 3 : 0) : length;
	// For UTF16 , backing of 1 unit then land right after 
	if (truncated && copyLength > 0) {
		wchar_t last = text[copyLength - 1];
		if (last >= 0xD800 && last <= 0xDBFF) {
			copyLength--;
		}
	}

	const wchar_t* dots = L"...";
	size_t dotslength = truncated ? wcslen(dots) : 0;
	size_t totalLength = copyLength + dotslength;
	wchar_t* preview = malloc((totalLength + 1) * sizeof(wchar_t));
	if (preview == NULL) { return NULL; }
	// Replace control characters with spaces for preview
	for (size_t i = 0; i < copyLength; i++) {
		wchar_t c = text[i];
		preview[i] = (c == L'\r' || c == L'\n' || c == L'\t') ? L' ' : c;
	}
	if (truncated) {
		wcscpy_s(preview + copyLength, dotslength + 1, dots);
	}
	else {
		preview[copyLength] = L'\0';
	}
	return preview;
}

void historyPopulateListBox(HWND listBoxHwnd, const ClipboardEntry* head) {
	if (listBoxHwnd == NULL) { return; }
	SendMessage(listBoxHwnd, LB_RESETCONTENT, 0, 0);
	// add to list box in reverse order
	for (const ClipboardEntry* entry = head; entry != NULL; entry = entry->next) {
		wchar_t* preview = historyPreviewBuild(entry->text);
		if (preview == NULL) {
			OutputDebugStringW(L"historyPreviewBuild: failed to build preview, skipping entry.\n");
			continue;
		}
		// preview to the list box and add the entry point init
		LRESULT index = SendMessage(listBoxHwnd, LB_ADDSTRING, 0, (LPARAM)preview);
		free(preview);
		if (index == LB_ERR || index == LB_ERRSPACE) {
			OutputDebugStringW(L"historyPopulateListBox: failed to add preview to list box.\n");
			continue;
		}
		SendMessage(listBoxHwnd, LB_SETITEMDATA, index, (LPARAM)entry);
	}
}