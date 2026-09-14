#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <Windows.h>
#include <stdbool.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SHOW_HISTORY 1000

bool trayIconCreate(HWND ownerHWND);
void trayIconDestroy(void);
void trayIconShowContextMenu(HWND ownerHWND);


#endif // !TRAY_ICON_H
