#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdbool.h>
#include <wchar.h>
#include <stddef.h>


typedef struct Settings {
	int ramHistoryCount;
	int dbHistoryCount;
	int searchResultLimit;
	unsigned int hotkeyModifiers;
	unsigned int hotkeyKey;
	bool startWithWindows;
} Settings;

bool settingsLoad(void);
bool settingsSave(void);
const Settings* settingsGet(void);
void settingsHotkeyText(wchar_t* buffer, size_t bufferSize);



#endif // !SETTINGS_H
