#include "session.h"
#include "database.h"
#include "path.h"
#include <stdlib.h>
#include <Windows.h>

static Database* db = NULL;

bool sessionInit(ClipboardEntry** head) {
	if (head == NULL) { return false; }
	*head = NULL;
	wchar_t* databasePath = NULL;
	if (!getLocalPath(&databasePath)) {
		OutputDebugStringW(L"sessionInit: failed to resolve database path.\n");
		return false;
	}

	db = databaseCreate(databasePath);
	free(databasePath);
	if (db == NULL) {
		OutputDebugStringW(L"sessionInit: failed to create database.\n");
		return false;
	}
	if (!databaseLoadH(db, head)) {
		OutputDebugStringW(L"sessionInit: failed to load existing history.\n");
		sessionShutdown();
		return false;
	}
	return true;
}

bool sessionSave(const ClipboardEntry* head) {
	if (db == NULL) { return false; }
	return databaseSaveH(db, head);
}

void sessionShutdown(void) {
	if (db != NULL) {
		databaseDestroy(db);
		db = NULL;
	}
}