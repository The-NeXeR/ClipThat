#ifndef SESSION_H
#define SESSION_H 


#include "clipboard.h"
#include <stdbool.h>

bool sessionInit(ClipboardEntry** head);
bool sessionTouch(const wchar_t* text, size_t maxDbCount);
bool sessionSearch(const wchar_t* query, int maxResults, ClipboardEntry** results);
void sessionShutdown(void);
bool sessionFlush(void);


#endif // !SESSION_H
