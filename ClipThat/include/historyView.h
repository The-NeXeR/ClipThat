#ifndef HISTORY_VIEW_h
#define HISTORY_VIEW_h

#include "clipboard.h"
#include <Windows.h>
#include <wchar.h>

wchar_t* historyPreviewBuild(const wchar_t* text);
void historyPopulateListBox(HWND listBoxHwnd, const ClipboardEntry* head);


#endif // !HISTORY_VIEW_h
