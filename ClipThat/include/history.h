#ifndef HISTORY_H
#define HISTORY_H

#include <stdbool.h>
#include "clipboard.h"

bool historyAdd(ClipboardEntry** head, const wchar_t* text);
void historyLimit(ClipboardEntry** head, size_t maxCount);
void historyClear(ClipboardEntry** head);
bool historyMatchesQuery(const wchar_t* text, const wchar_t* query);

#endif // !HISTORY_H
