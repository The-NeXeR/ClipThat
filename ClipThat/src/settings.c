#include "settings.h"
#include <Windows.h>
#include "path.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define DEFAULT_RAM_HISTORY_COUNT 50
#define DEFAULT_DB_HISTORY_COUNT 15000
#define DEFAULT_SEARCH_RESULT_LIMIT 1000
#define DEFAULT_HOTKEY_MODIFIERS (MOD_CONTROL | MOD_ALT)
#define DEFAULT_HOTKEY_KEY 0x48

static Settings current = {
	DEFAULT_RAM_HISTORY_COUNT,
	DEFAULT_DB_HISTORY_COUNT,
	DEFAULT_SEARCH_RESULT_LIMIT,
	DEFAULT_HOTKEY_MODIFIERS,
	DEFAULT_HOTKEY_KEY,
	true
};

static char asciiLowerCase(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + ('a' - 'A')) : c; }

static bool equalIgnoreCase(const char* a, const char* b) {
	while (*a != '\0' && *b != '\0') {
		if (asciiLowerCase(*a) != asciiLowerCase(*b)) return false;
		a++;
		b++;
	}
	return *a == '\0' && *b == '\0';
}

static bool parseDigits(const char* text, long* outValue) {
	if (text == NULL || *text == '\0') return false;
	long value = 0;
	for (const char* ptr = text; *ptr != '\0'; ptr++) {
		if (*ptr < '0' || *ptr > '9') return false;
		value = value * 10 + (*ptr - '0');
		if (value > 100000) return false;
	}
	*outValue = value;
	return true;
}

static bool parseBoolean(const char* value, bool* outValue) {
	if (equalIgnoreCase(value, "true") || equalIgnoreCase(value, "yes") || strcmp(value, "1") == 0) {
		*outValue = true;
		return true;
	}
	if (equalIgnoreCase(value, "false") || equalIgnoreCase(value, "no") || strcmp(value, "0") == 0) {
		*outValue = false;
		return true;
	}
	return false;
}

static bool parseModifiers(const char* value, unsigned int* outValue) {
	long number = 0;
	if (parseDigits(value, &number)) {
		unsigned int mask = (unsigned int)number & 0xF;
		if (mask == 0) return false;
		*outValue = mask;
		return true;
	}
	unsigned int result = 0;
	const char* ptr = value;
	while (*ptr != '\0') {
		while (*ptr == '+' || *ptr == ',' || *ptr == ' ' || *ptr == '\t') { ptr++; }
		if (*ptr == '\0') { break; }
		const char* word = ptr;
		while (*ptr != '\0' && *ptr != '+' && *ptr != ',' && *ptr != ' ' && *ptr != '\t') { ptr++; }
		size_t wordLength = (size_t)(ptr - word);
		char wordBuffer[16];
		if (wordLength == 0 || wordLength >= sizeof(wordBuffer)) return false;
		for (size_t i = 0; i < wordLength; i++) { wordBuffer[i] = asciiLowerCase(word[i]); }
		wordBuffer[wordLength] = '\0';
		if (strcmp(wordBuffer, "ctrl") == 0 || strcmp(wordBuffer, "control") == 0) result |= MOD_CONTROL;
		else if (strcmp(wordBuffer, "alt") == 0) result |= MOD_ALT;
		else if (strcmp(wordBuffer, "shift") == 0) result |= MOD_SHIFT;
		else if (strcmp(wordBuffer, "win") == 0 || strcmp(wordBuffer, "windows") == 0) result |= MOD_WIN;
		else return false;
	}
	if (result == 0) return false;
	*outValue = result;
	return true;
}

static bool parseKeys(const char* value, unsigned int* outValue) {
	long number = 0;
	if (parseDigits(value, &number)) {
		if (number >= 0x08 && number <= 0xFE) {
			*outValue = (unsigned int)number;
			return true;
		}
		return false;
	}
	size_t length = strlen(value);
	if (length >= 2 && (value[0] == 'F' || value[0] == 'f')) {
		if (parseDigits(value + 1, &number) && number >= 1 && number <= 24) {
			*outValue = (unsigned int)(0x70 + number - 1);
			return true;
		}
		return false;
	}
	if (length == 1) {
		char c = value[0];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			*outValue = (unsigned int)(c >= 'a' ? c - 32 : c);
			return true;
		}
		if (c >= '0' && c <= '9') {
			*outValue = (unsigned int)c;
			return true;
		}
	}
	return false;
}

static void applyCount(const char* value, int* target, int fallback, int minimum, int maximum) {
	long parsed = 0;
	if (parseDigits(value, &parsed) && parsed >= minimum && parsed <= maximum) { *target = (int)parsed; }
	else { *target = fallback; }
}

static void applyLine(const char* line) {
	while (*line == ' ' || *line == '\t') { line++; }
	if (*line == '\0' || *line == '#' || *line == ';') { return; }
	const char* equals = strchr(line, '=');
	if (equals == NULL) { return; }
	size_t nameLength = (size_t)(equals - line);
	char name[32];
	if (nameLength == 0 || nameLength >= sizeof(name)) { return; }
	memcpy(name, line, nameLength);
	name[nameLength] = '\0';
	size_t start = 0;
	while (name[start] == ' ' || name[start] == '\t') { start++; }
	if (start > 0) { memmove(name, name + start, strlen(name) - start + 1); }
	size_t end = strlen(name);
	while (end > 0 && (name[end - 1] == ' ' || name[end - 1] == '\t')) { end--; }
	name[end] = '\0';
	if (name[0] == '\0') { return; }
	for (char* p = name; *p != '\0'; p++) { *p = asciiLowerCase(*p); }

	const char* value = equals + 1;
	while (*value == ' ' || *value == '\t') { value++; }
	size_t valueLength = strlen(value);
	while (valueLength > 0 && (value[valueLength - 1] == ' ' || value[valueLength - 1] == '\t')) { valueLength--; }
	char valueBuffer[256];
	if (valueLength >= sizeof(valueBuffer)) { return; }
	memcpy(valueBuffer, value, valueLength);
	valueBuffer[valueLength] = '\0';

	if (strcmp(name, "ramhistorycount") == 0) {
		applyCount(valueBuffer, &current.ramHistoryCount, DEFAULT_RAM_HISTORY_COUNT, 5, 500);
	}
	else if (strcmp(name, "dbhistorycount") == 0) {
		applyCount(valueBuffer, &current.dbHistoryCount, DEFAULT_DB_HISTORY_COUNT, 100, 100000);
	}
	else if (strcmp(name, "searchresultlimit") == 0) {
		applyCount(valueBuffer, &current.searchResultLimit, DEFAULT_SEARCH_RESULT_LIMIT, 50, 5000);
	}
	else if (strcmp(name, "startwithwindows") == 0) {
		if (!parseBoolean(valueBuffer, &current.startWithWindows)) { current.startWithWindows = true; }
	}
	else if (strcmp(name, "hotkeymodifiers") == 0) {
		if (!parseModifiers(valueBuffer, &current.hotkeyModifiers)) { current.hotkeyModifiers = DEFAULT_HOTKEY_MODIFIERS; }
	}
	else if (strcmp(name, "hotkeykey") == 0) {
		if (!parseKeys(valueBuffer, &current.hotkeyKey)) { current.hotkeyKey = DEFAULT_HOTKEY_KEY; }
	}
}

static bool readFile(const wchar_t* path, char** data, size_t* length) {
	FILE* file = NULL;
	if (_wfopen_s(&file, path, L"rb") != 0 || file == NULL) { return false; }
	if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return false; }
	long size = ftell(file);
	if (size < 0 || size > 65536) { fclose(file); return false; }
	rewind(file);
	char* buffer = malloc((size_t)size + 1);
	if (buffer == NULL) { fclose(file); return false; }
	size_t read = fread(buffer, 1, (size_t)size, file);
	fclose(file);
	buffer[read] = '\0';
	*data = buffer;
	*length = read;
	return true;
}

static void buildModifiersText(unsigned int modifiers, char* buffer, size_t bufferSize) {
	const char* labels[4] = { "Ctrl", "Alt", "Shift", "Win" };
	const unsigned int flags[4] = { MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN };
	size_t offset = 0;
	for (int i = 0; i < 4; i++) {
		if ((modifiers & flags[i]) != 0) {
			if (offset > 0 && offset < bufferSize) { buffer[offset++] = '+'; }
			size_t labelLength = strlen(labels[i]);
			if (offset + labelLength < bufferSize) {
				memcpy(buffer + offset, labels[i], labelLength);
				offset += labelLength;
			}
		}
	}
	buffer[offset] = '\0';
}

static void buildKeyText(unsigned int key, char* buffer, size_t bufferSize) {
	if ((key >= 0x30 && key <= 0x39) || (key >= 0x41 && key <= 0x5A)) {
		buffer[0] = (char)key;
		buffer[1] = '\0';
		return;
	}
	if (key >= 0x70 && key <= 0x87) {
		sprintf_s(buffer, bufferSize, "F%u", key - 0x70 + 1);
		return;
	}
	sprintf_s(buffer, bufferSize, "%u", key);
}



bool settingsLoad(void) {
	wchar_t* path = NULL;
	if (!getLocalFilePath(L"settings.ini", &path)) { return false; }
	char* data = NULL;
	size_t length = 0;
	bool existed = readFile(path, &data, &length);
	free(path);
	if (existed) {
		const char* cursor = data;
		const char* fileEnd = data + length;
		while (cursor < fileEnd) {
			const char* lineEnd = memchr(cursor, '\n', (size_t)(fileEnd - cursor));
			size_t lineLength = (lineEnd != NULL) ? (size_t)(lineEnd - cursor) : (size_t)(fileEnd - cursor);
			if (lineLength < 512) {
				char line[512];
				memcpy(line, cursor, lineLength);
				line[lineLength] = '\0';
				size_t trim = lineLength;
				while (trim > 0 && (line[trim - 1] == '\r' || line[trim - 1] == ' ' || line[trim - 1] == '\t')) { trim--; }
				line[trim] = '\0';
				applyLine(line);
			}
			cursor = (lineEnd != NULL) ? lineEnd + 1 : fileEnd;
		}
		free(data);
	}
	else {
		settingsSave();
	}
	return existed;
}

bool settingsSave(void) {
	wchar_t* path = NULL;
	if (!getLocalFilePath(L"settings.ini", &path)) { return false; }
	size_t pathLength = wcslen(path);
	wchar_t* tempPath = malloc((pathLength + 5) * sizeof(wchar_t));
	if (tempPath == NULL) { free(path); return false; }
	wcscpy_s(tempPath, pathLength + 5, path);
	wcscat_s(tempPath, pathLength + 5, L".tmp");
	FILE* file = NULL;
	if (_wfopen_s(&file, tempPath, L"wb") != 0 || file == NULL) {
		free(tempPath);
		free(path);
		return false;
	}
	char modifiersText[32];
	char keyText[16];
	buildModifiersText(current.hotkeyModifiers, modifiersText, sizeof(modifiersText));
	buildKeyText(current.hotkeyKey, keyText, sizeof(keyText));
	int written = fprintf(file,
		"# ClipThat settings - edit and restart to apply\r\n"
		"ramHistoryCount=%d\r\n"
		"dbHistoryCount=%d\r\n"
		"searchResultLimit=%d\r\n"
		"startWithWindows=%s\r\n"
		"hotkeyModifiers=%s\r\n"
		"hotkeyKey=%s\r\n",
		current.ramHistoryCount,
		current.dbHistoryCount,
		current.searchResultLimit,
		current.startWithWindows ? "true" : "false",
		modifiersText,
		keyText);
	bool ok = (written > 0);
	if (fclose(file) != 0) { ok = false; }
	if (ok) {
		ok = MoveFileExW(tempPath, path, MOVEFILE_REPLACE_EXISTING) != 0;
	}
	if (!ok) { DeleteFileW(tempPath); }
	free(tempPath);
	free(path);
	return ok;
}


const Settings* settingsGet(void) {
	return &current;
}

void settingsHotkeyText(wchar_t* buffer, size_t bufferSize) {
	char modifiersText[32];
	char keyText[16];
	wchar_t wideModifiers[32];
	wchar_t wideKey[16];
	buildModifiersText(current.hotkeyModifiers, modifiersText, sizeof(modifiersText));
	buildKeyText(current.hotkeyKey, keyText, sizeof(keyText));
	size_t modifiersLength = strlen(modifiersText);
	size_t keyLength = strlen(keyText);
	for (size_t i = 0; i < modifiersLength; i++) { wideModifiers[i] = (wchar_t)modifiersText[i]; }
	wideModifiers[modifiersLength] = L'\0';
	for (size_t i = 0; i < keyLength; i++) { wideKey[i] = (wchar_t)keyText[i]; }
	wideKey[keyLength] = L'\0';
	swprintf_s(buffer, bufferSize, L"%s+%s", wideModifiers, wideKey);
}
