#include "startup.h"
#include <Windows.h>

#define RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RUN_VALUE L"ClipThat"


static bool getPath(wchar_t* buffer, DWORD bChar) {
	DWORD write = GetModuleFileNameW(NULL, buffer, bChar);
	if (write == 0 || write >= bChar) return false;
	return true;
}

bool startupEnable(void) {
	wchar_t exePath[MAX_PATH] = { 0 };
	if (!getPath(exePath, MAX_PATH)) return false;
	
	size_t len = wcslen(exePath);
	if (len + 3 > MAX_PATH) return false;

	wchar_t command[MAX_PATH] = { 0 };
	command[0] = L'"';
	wcscpy_s(command + 1, MAX_PATH - 1, exePath);
	command[len + 1] = L'"';
	command[len + 2] = L'\0';

	HKEY key = NULL;
	LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
	if (result != ERROR_SUCCESS) return false;
	result = RegSetValueExW(key, RUN_VALUE, 0, REG_SZ, (const BYTE*)command, (DWORD)((len + 3) * sizeof(wchar_t)));
	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}


bool startupDisable(void) {
	LONG result = RegDeleteKeyValueW(HKEY_CURRENT_USER, RUN_KEY, RUN_VALUE);
	return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

bool startupIsEnabled(void) {
	HKEY key = NULL;
	LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS) return false;
	result = RegQueryValueExW(key, RUN_VALUE, NULL, NULL, NULL, NULL);
	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}
