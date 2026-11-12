#ifndef DATABASE_H
#define DATABASE_H

#include <wchar.h>
#include <stdbool.h>
#include "sqlite3.h"
#include "clipboard.h"

typedef struct Database {
	sqlite3* handle; 
	wchar_t* path;
} Database;

// database cycle
Database* databaseCreate(const wchar_t* path);
void databaseDestroy(Database* db);
bool databaseOpen(Database* db);
void databaseClose(Database* db);

// database setup
bool databaseInit(Database* db);

// database save/load
bool databaseSaveH(Database* db, const ClipboardEntry* head);
bool databaseLoadH(Database* db, ClipboardEntry** head);

#endif // !DATABASE_H
