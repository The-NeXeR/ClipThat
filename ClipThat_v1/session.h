#ifndef SESSION_H
#define SESSION_H 


#include "clipboard.h"
#include <stdbool.h>

bool sessionInit(ClipboardEntry** head);
bool sessionSave(const ClipboardEntry* head);
void sessionShutdown(void);



#endif // !SESSION_H
