#include "path.h"
#include <ShlObj.h>
#include <stdlib.h>
#include <wchar.h>

bool getLocalPath(wchar_t** path) {
	if (path == NULL) { return false; }
	*path = NULL;
	PWSTR localPath = NULL;
	HRESULT hr = SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &localPath);
	if (FAILED(hr)) { return false; }
	const wchar_t* databaseFolder = L"ClipThat";
	const wchar_t* databaseFile = L"clipThat.db";

	size_t baseLength = wcslen(localPath);
	size_t folderLength = wcslen(databaseFolder);
	size_t fileLength = wcslen(databaseFile);
	size_t dirPathLength = baseLength + 1 + folderLength;
	size_t dataPathLength = dirPathLength + 1 + fileLength;
	wchar_t* dirPath = malloc((dirPathLength + 1) * sizeof(*dirPath));

	if (dirPath == NULL) {
		CoTaskMemFree(localPath);
		return false;
	}
	wcscpy_s(dirPath, dirPathLength + 1, localPath);
	wcscat_s(dirPath, dirPathLength + 1, L"\\");
	wcscat_s(dirPath, dirPathLength + 1, databaseFolder);
	if (!CreateDirectoryW(dirPath, NULL)) {
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS) {
			free(dirPath);
			CoTaskMemFree(localPath);
			return false;
		}
	}
	wchar_t* dataBasePath = malloc((dataPathLength + 1) * sizeof(*dataBasePath));
	if (dataBasePath == NULL) {
		free(dirPath);
		CoTaskMemFree(localPath);
		return false;
	}

	wcscpy_s(dataBasePath, dataPathLength + 1, dirPath);
	wcscat_s(dataBasePath, dataPathLength + 1, L"\\");
	wcscat_s(dataBasePath, dataPathLength + 1, databaseFile);
	free(dirPath);
	CoTaskMemFree(localPath);
	*path = dataBasePath;
	return true;
}