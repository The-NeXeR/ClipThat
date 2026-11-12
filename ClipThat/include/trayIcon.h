#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <Windows.h>
#include <stdbool.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SHOW_HISTORY 1000
#define ID_TRAY_FLUSH_HISTORY 1002

bool trayIconCreate(HWND ownerHWND);
void trayIconDestroy(void);
void trayIconRestore(void);
void trayIconShowContextMenu(HWND ownerHWND);
UINT trayIconTaskbarCreatedMessage(void);


#endif // !TRAY_ICON_H
