#ifndef PATH_H
#define PATH_H

#include <stdbool.h>
#include <wchar.h>

bool getLocalPath(wchar_t** path);
bool getLocalFilePath(const wchar_t* fileName, wchar_t** path);

#endif // !PATH_H
