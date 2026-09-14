#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stddef.h>
#include <wchar.h>
#include <stdbool.h>
#include <Windows.h>


typedef struct ClipboardEntry {
    wchar_t* text;
    size_t length;
    struct ClipboardEntry* next;
} ClipboardEntry;

void clipboardFreeHistory(ClipboardEntry** head);
bool clipboardHandleUpdate(HWND owner, ClipboardEntry** head, size_t maxHistoryCount);
bool clipboardSetText(HWND owner, const wchar_t* text);

#endif