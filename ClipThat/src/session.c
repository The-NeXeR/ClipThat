#include "session.h"
#include "database.h"
#include "path.h"
#include "config.h"
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
	if (!databaseLoadH(db, head, MAX_HISTORY_COUNT)) {
		OutputDebugStringW(L"sessionInit: failed to load existing history.\n");
		sessionShutdown();
		return false;
	}
	return true;
}
bool sessionFlush(void) {
	if (db == NULL) { return false; }
	return databaseFlushH(db);
}

bool sessionTouch(const wchar_t* text, size_t maxDbCount) {
	if (db == NULL) { return false; }
	return databaseTouchH(db, text, (int)maxDbCount);
}

bool sessionSearch(const wchar_t* query, int maxResults, ClipboardEntry** results) {
	if (db == NULL) { return false; }
	return databaseSearchH(db, query, maxResults, results);
}

void sessionShutdown(void) {
	if (db != NULL) {
		databaseDestroy(db);
		db = NULL;
	}
}