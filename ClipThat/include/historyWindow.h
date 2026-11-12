#ifndef HISTORY_WINDOW_H
#define HISTORY_WINDOW_H
#include <Windows.h>
#include <stdbool.h>
#include "clipboard.h"

bool historyWindowCreate(HINSTANCE hInstance);
void historyWindowRefresh(const ClipboardEntry* head);
void historyWindowShow(const ClipboardEntry* head, HWND targetWindow);

#endif // !HISTORY_WINDOW_H
