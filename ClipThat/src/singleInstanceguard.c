#include "singleInstanceguard.h"
#include <windows.h>

#define MUTEX L"Local\\ClipThat.InstanceGuard"

static HANDLE mutexHandle = NULL;

bool singleInstanceGuard(void) {
	mutexHandle = CreateMutexW(NULL, FALSE, MUTEX);
	if (mutexHandle == NULL) { return false; }
	return GetLastError() == ERROR_ALREADY_EXISTS;
}

void singleInstanceRelease(void) {
	if (mutexHandle != NULL) {
		CloseHandle(mutexHandle);
		mutexHandle = NULL;
	}
}