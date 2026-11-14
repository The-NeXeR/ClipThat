#ifndef SINGLEINSTANCEGUARD_H
#define SINGLEINSTANCEGUARD_H
#include <stdbool.h>


bool singleInstanceGuard(void);
void singleInstanceRelease(void);

#endif // !SINGLEINSTANCEGUARD_H
