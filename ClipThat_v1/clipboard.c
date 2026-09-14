#include "clipboard.h"
#include "history.h"
#include <Windows.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#include <string.h>

static UINT clipboardSeq = 0;
static bool clipboardPending = false;

static bool clipboardOpen(HWND owner) {
	enum {MAX_ATTEMP = 5,
	RETRY_DELAY = 10 // ms
	};
	for (unsigned attempts = 0; attempts < MAX_ATTEMP; ++attempts) {
		if (OpenClipboard(owner)) {
			return true;
		}
		if (attempts + 1 < MAX_ATTEMP) {
			Sleep(RETRY_DELAY);
		}
	}
	OutputDebugStringW(L"clipboardOpenWithRetry: failed to open clipboard after retries.\n");
	return false;
}

bool clipboardSetText(HWND owner, const wchar_t* text) {
	if (text == NULL) { return false; }
	if (!clipboardOpen(owner)) { return false; }
	bool success = false;
	HGLOBAL gMem = NULL;
	wchar_t* clipText = NULL;
	size_t length = wcslen(text);
	if (length > (SIZE_MAX) / sizeof(wchar_t) - 1) { goto cleanup; }
	if (!EmptyClipboard()) { goto cleanup; }
	SIZE_T sizealloc = (SIZE_T)(length + 1) * sizeof(wchar_t);
	gMem = GlobalAlloc(GMEM_MOVEABLE, sizealloc);
	if (gMem == NULL) { goto cleanup; }

	clipText = GlobalLock(gMem);
	if (clipText == NULL) { goto cleanup; }
	memcpy(clipText, text, sizealloc);
	if (!GlobalUnlock(gMem)) {
		DWORD err = GetLastError();
		if (err != NO_ERROR) { goto cleanup; }
	}
	clipText = NULL;
	if (SetClipboardData(CF_UNICODETEXT, gMem) == NULL) { goto cleanup; }
	gMem = NULL;

	clipboardSeq = GetClipboardSequenceNumber();
	clipboardPending = true;
	success = true;

cleanup:
	if (clipText != NULL) { GlobalUnlock(gMem); }
	if (gMem != NULL) { GlobalFree(gMem); }
	CloseClipboard();
	return success;
}
bool clipboardHandleUpdate(HWND owner, ClipboardEntry** head, size_t maxHistoryCount) {
	if (head == NULL) { return false; }
	UINT currentSeq = GetClipboardSequenceNumber();
	if (clipboardPending && currentSeq == clipboardSeq) {
		clipboardPending = false;
		return true;
	}
	clipboardPending = false;
	if (!clipboardOpen(owner)) {
		return false;
	}
	bool success = false;
	HANDLE dataHandle = GetClipboardData(CF_UNICODETEXT);
	if (dataHandle == NULL) { CloseClipboard(); return true; }
	wchar_t* text = GlobalLock(dataHandle);
	if (text == NULL) { CloseClipboard(); return false; }
	if (!historyAdd(head, text)) {
		GlobalUnlock(dataHandle);
		CloseClipboard();
		return false;
	}
	historyLimit(head, maxHistoryCount);
	GlobalUnlock(dataHandle);
	CloseClipboard();
	success = true;
	return success;
}