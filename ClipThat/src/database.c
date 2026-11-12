#include "database.h"
#include "sqlite3.h"
#include <stdio.h>
#include <windows.h>
#include "history.h"



Database* databaseCreate(const wchar_t* path) {
	if (!path) return NULL;

	Database* db = calloc(1, sizeof(Database));
	if (!db) return NULL;

	size_t pLen = wcslen(path) + 1;
	db->path = malloc(pLen * sizeof(wchar_t));
	if (!db->path) {
		free(db);
		return NULL;
	}
	wcscpy_s(db->path, pLen, path);

	if (!databaseOpen(db)) {
		databaseDestroy(db);
		return NULL;
	}
	if (!databaseInit(db)) {
		databaseDestroy(db);
		return NULL;
	}
	return db;
}

void databaseDestroy(Database* db) {
	if (!db) return;
	databaseClose(db);
	free(db->path);
	db->path = NULL;
	free(db);
}
bool databaseOpen(Database* db) {
	if (!db || !db->path) return false;
	if (db->handle != NULL) return true; // already open
	// SQLITE_OPEN16, Windows uses UTF-16, so we can use the path directly
	int rc = sqlite3_open16(db->path, &db->handle);
	if (rc != SQLITE_OK) {
		if (db->handle != NULL)
		{
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_close(db->handle);
			db->handle = NULL;
		}
		return false;
	}

	// WAL mode is enabled for better performance
	rc = sqlite3_exec(db->handle, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);

	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite WAL mode failed.\n");
		databaseClose(db);
		return false;
	}
	rc = sqlite3_exec(db->handle, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite Foreign key failed.\n");
		databaseClose(db);
		return false;
	}

	rc = sqlite3_exec(db->handle, "PRAGMA busy_timeout = 5000;", NULL, NULL, NULL);

	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite busy timeout failed.\n");
		databaseClose(db);
		return false;
	}

	rc = sqlite3_exec(db->handle, "PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);

	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite synchronous mode failed.\n");
		databaseClose(db);
		return false;
	}


	return true;
}

void databaseClose(Database* db) {
	if (!db || !db->handle) return;
	int retCode = sqlite3_close_v2(db->handle);
	if (retCode != SQLITE_OK) {
		OutputDebugStringA("SQLite close failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return;
	}
	db->handle = NULL;
}


/*

	Schema for the database:
	Fuck SQLite, and its Documentation.

*/

static const char* schemaV2 =
	"CREATE TABLE IF NOT EXISTS cHistory ("
	"id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"text TEXT NOT NULL UNIQUE,"
	"cDate INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
	"lastUsed INTEGER NOT NULL DEFAULT (strftime('%s','now'))"
	");";

static bool databaseSetVersion(Database* db, int version) {
	char sql[64];
	sprintf_s(sql, sizeof(sql), "PRAGMA user_version = %d;", version);
	int rc = sqlite3_exec(db->handle, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite version stamp failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return false;
	}
	return true;
}

static bool databaseGetVersion(Database* db, int* version) {
	sqlite3_stmt* stmt = NULL;
	if (sqlite3_prepare_v2(db->handle, "PRAGMA user_version;", -1, &stmt, NULL) != SQLITE_OK) {
		return false;
	}
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		*version = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);
	return true;
}

static bool databaseTableExists(Database* db, bool* exists) {
	sqlite3_stmt* stmt = NULL;
	if (sqlite3_prepare_v2(db->handle, "SELECT 1 FROM sqlite_master WHERE type='table' AND name='cHistory';", -1, &stmt, NULL) != SQLITE_OK) {
		return false;
	}
	*exists = (sqlite3_step(stmt) == SQLITE_ROW);
	sqlite3_finalize(stmt);
	return true;
}

static bool databaseMigrateV1(Database* db) {
	const char* migration =
		"BEGIN TRANSACTION;"
		"CREATE TABLE cHistory_v2 ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"text TEXT NOT NULL UNIQUE,"
		"cDate INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
		"lastUsed INTEGER NOT NULL DEFAULT (strftime('%s','now'))"
		");"
		"INSERT INTO cHistory_v2 (text, cDate, lastUsed) "
		"SELECT text, cDate, id FROM cHistory ORDER BY id;"
		"DROP TABLE cHistory;"
		"ALTER TABLE cHistory_v2 RENAME TO cHistory;"
		"COMMIT;";
	char* errMessage = NULL;
	int rc = sqlite3_exec(db->handle, migration, NULL, NULL, &errMessage);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("Database migration failed: ");
		if (errMessage != NULL) {
			OutputDebugStringA(errMessage);
			OutputDebugStringA("\n");
			sqlite3_free(errMessage);
		}
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	return databaseSetVersion(db, 2);
}

bool databaseInit(Database* db) {
	if (!db || !db->handle) return false;

	int version = 0;
	if (!databaseGetVersion(db, &version)) { return false; }

	bool tableExists = false;
	if (!databaseTableExists(db, &tableExists)) { return false; }

	if (tableExists && version < 2) {
		return databaseMigrateV1(db);
	}

	if (!tableExists) {
		char* errMessage = NULL;
		int rc = sqlite3_exec(db->handle, schemaV2, NULL, NULL, &errMessage);
		if (rc != SQLITE_OK) {
			OutputDebugStringA("Database initialization failed: ");
			if (errMessage != NULL) {
				OutputDebugStringA(errMessage);
				OutputDebugStringA("\n");
				sqlite3_free(errMessage);
			}
			return false;
		}
		return databaseSetVersion(db, 2);
	}

	return true;
}

bool databaseTouchH(Database* db, const wchar_t* text, int maxCount) {
	if (!db || !db->handle || !text || text[0] == L'\0' || maxCount < 1) return false;

	const char* upsertSql =
		"INSERT INTO cHistory (text, cDate, lastUsed) "
		"VALUES (?1, strftime('%s','now'), strftime('%s','now')) "
		"ON CONFLICT(text) DO UPDATE SET lastUsed = strftime('%s','now');";

	sqlite3_stmt* stmt = NULL;
	int rc = sqlite3_prepare_v2(db->handle, upsertSql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite upsert prepare failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return false;
	}
	rc = sqlite3_bind_text16(stmt, 1, text, -1, SQLITE_TRANSIENT);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite upsert bind failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_finalize(stmt);
		return false;
	}
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		OutputDebugStringA("SQLite upsert step failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);

	const char* pruneSql =
		"DELETE FROM cHistory WHERE id NOT IN "
		"(SELECT id FROM cHistory ORDER BY lastUsed DESC LIMIT ?1);";

	stmt = NULL;
	rc = sqlite3_prepare_v2(db->handle, pruneSql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite prune prepare failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return false;
	}
	rc = sqlite3_bind_int(stmt, 1, maxCount);
	if (rc != SQLITE_OK) {
		OutputDebugStringA("SQLite prune bind failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_finalize(stmt);
		return false;
	}
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		OutputDebugStringA("SQLite prune step failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);

	return true;
}

bool databaseSaveH(Database* db, const ClipboardEntry* head) {
	if (!db || !db->handle) return false;

	int returnSQL = sqlite3_exec(db->handle, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite transaction begin failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return false;
	}

	returnSQL = sqlite3_exec(db->handle, "DELETE FROM cHistory;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite delete failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	const char* sql = "INSERT INTO cHistory (text, cDate, lastUsed) VALUES (?1, ?2, ?3);";
	sqlite3_stmt* stmt = NULL;

	returnSQL = sqlite3_prepare_v2(db->handle, sql, -1, &stmt, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite prepare failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}

	//I don't know if it's gonna be optimized but for SQLite return code audit is important
	for (const ClipboardEntry* entry = head; entry != NULL; entry = entry->next) {
		returnSQL = sqlite3_bind_text16(stmt, 1, entry->text, -1, SQLITE_TRANSIENT);
		if (returnSQL != SQLITE_OK) {
			OutputDebugStringA("SQLite bind failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
		returnSQL = sqlite3_bind_int64(stmt, 2, entry->firstCopied);
		if (returnSQL != SQLITE_OK) {
			OutputDebugStringA("SQLite bind failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
		returnSQL = sqlite3_bind_int64(stmt, 3, entry->lastUsed);
		if (returnSQL != SQLITE_OK) {
			OutputDebugStringA("SQLite bind failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
		returnSQL = sqlite3_step(stmt);
		if (returnSQL != SQLITE_DONE) {
			OutputDebugStringA("SQLite step failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
		returnSQL = sqlite3_reset(stmt);
		if (returnSQL != SQLITE_OK) {
			OutputDebugStringA("SQLite reset failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
		returnSQL = sqlite3_clear_bindings(stmt);
		if (returnSQL != SQLITE_OK) {
			OutputDebugStringA("SQLite clear bindings failed: ");
			OutputDebugStringA(sqlite3_errmsg(db->handle));
			sqlite3_finalize(stmt);
			sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
			return false;
		}
	}

	sqlite3_finalize(stmt);
	returnSQL = sqlite3_exec(db->handle, "COMMIT;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite commit failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		OutputDebugStringA("\n");
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	return true;
}

bool databaseLoadH(Database* db, ClipboardEntry** head) {
	if (!db || !db->handle || !head) return false;

	*head = NULL;
	ClipboardEntry* tail = NULL;

	sqlite3_stmt* stmt = NULL;
	const char* sql = "SELECT text, cDate, lastUsed FROM cHistory ORDER BY lastUsed DESC;";

	int rc = sqlite3_prepare_v2(db->handle, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) { return false; }
	bool done = true;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const wchar_t* content = (const wchar_t*)sqlite3_column_text16(stmt, 0);
		if (content == NULL) {
			done = false;
			break;
		}
		ClipboardEntry* entry = calloc(1, sizeof(*entry));
		if (!entry) { done = false; break; }
		size_t len = wcslen(content) + 1;
		entry->text = malloc(len * sizeof(wchar_t));
		if (!entry->text) {
			free(entry);
			done = false;
			break;
		}
		wcscpy_s(entry->text, len, content);
		entry->length = len - 1;
		entry->firstCopied = sqlite3_column_int64(stmt, 1);
		entry->lastUsed = sqlite3_column_int64(stmt, 2);
		entry->next = NULL;

		if (!*head) *head = entry;
		else tail->next = entry;
		tail = entry;
	}

	sqlite3_finalize(stmt);

	if (!done || rc != SQLITE_DONE) {
		historyClear(head);
		return false;
	}

	return true;
}

bool databaseFlushH(Database* db) {
	if (!db || !db->handle) return false;
	int returnSQL = sqlite3_exec(db->handle, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite transaction begin failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		return false;
	}
	returnSQL = sqlite3_exec(db->handle, "DELETE FROM cHistory;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite delete failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	returnSQL = sqlite3_exec(db->handle, "COMMIT;", NULL, NULL, NULL);
	if (returnSQL != SQLITE_OK) {
		OutputDebugStringA("SQLite commit failed: ");
		OutputDebugStringA(sqlite3_errmsg(db->handle));
		OutputDebugStringA("\n");
		sqlite3_exec(db->handle, "ROLLBACK;", NULL, NULL, NULL);
		return false;
	}
	return true;
}
