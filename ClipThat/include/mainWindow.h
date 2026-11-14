#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Windows.h>
#include <stdbool.h>
#define WM_CLIPTHAT_SHOW (WM_APP + 1)
#define MAIN_WINDOW_CLASS_NAME L"MyWindow"

bool mainWindowCreate(HINSTANCE hInstance);

#endif // !MAIN_WINDOW_H
