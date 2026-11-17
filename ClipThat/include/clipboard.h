#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stddef.h>
#include <wchar.h>
#include <stdbool.h>
#include <Windows.h>


typedef struct ClipboardEntry {
    wchar_t* text;
    size_t length;
    long long firstCopied;
    long long lastUsed;
    struct ClipboardEntry* next;
} ClipboardEntry;

bool clipboardHandleUpdate(HWND owner, ClipboardEntry** head, size_t maxHistoryCount, size_t maxEntryChars, bool* entrySkipped);
bool clipboardSetText(HWND owner, const wchar_t* text);
void clipboardPause(bool paused);
bool clipboardPauseStatus(void);

#endif
