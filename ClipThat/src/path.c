#include "path.h"
#include <ShlObj.h>
#include <stdlib.h>
#include <wchar.h>


static bool getLocalDir(wchar_t** dir) {
	if (dir == NULL) { return false; }
	*dir = NULL;
	PWSTR localPath = NULL;
	HRESULT hr = SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &localPath);
	if (FAILED(hr)) { return false; }
	const wchar_t* folderName = L"ClipThat";
	size_t baseLength = wcslen(localPath);
	size_t folderLength = wcslen(folderName);
	size_t dirLength = baseLength + 1 + folderLength;
	wchar_t* dirPath = malloc((dirLength + 1) * sizeof(*dirPath));
	if (dirPath == NULL) {
		CoTaskMemFree(localPath);
		return false;
	}
	wcscpy_s(dirPath, dirLength + 1, localPath);
	wcscat_s(dirPath, dirLength + 1, L"\\");
	wcscat_s(dirPath, dirLength + 1, folderName);
	CoTaskMemFree(localPath);
	if (!CreateDirectoryW(dirPath, NULL)) {
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS) {
			free(dirPath);
			return false;
		}
	}
	*dir = dirPath;
	return true;
}

bool getLocalFilePath(const wchar_t* fileName, wchar_t** path) {
	if (fileName == NULL || path == NULL) { return false; }
	wchar_t* dirPath = NULL;
	if (!getLocalDir(&dirPath)) { return false; }
	size_t dirLength = wcslen(dirPath);
	size_t fileLength = wcslen(fileName);
	size_t totalLength = dirLength + 1 + fileLength;
	wchar_t* fullPath = malloc((totalLength + 1) * sizeof(*fullPath));
	if (fullPath == NULL) {
		free(dirPath);
		return false;
	}
	wcscpy_s(fullPath, totalLength + 1, dirPath);
	wcscat_s(fullPath, totalLength + 1, L"\\");
	wcscat_s(fullPath, totalLength + 1, fileName);
	free(dirPath);
	*path = fullPath;
	return true;
}


bool getLocalPath(wchar_t** path) {
	return getLocalFilePath(L"clipThat.db", path);
}