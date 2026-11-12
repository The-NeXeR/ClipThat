#ifndef SESSION_H
#define SESSION_H 


#include "clipboard.h"
#include <stdbool.h>

bool sessionInit(ClipboardEntry** head);
bool sessionSave(const ClipboardEntry* head);
bool sessionTouch(const wchar_t* text, size_t maxCount);
void sessionShutdown(void);
bool sessionFlush(void);


#endif // !SESSION_H
