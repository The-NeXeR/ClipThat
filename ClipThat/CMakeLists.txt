cmake_minimum_required(VERSION 3.15)

project(ClipThat
    VERSION 1.0.1
    DESCRIPTION "Lightweight clipboard manager for Windows"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    # Additional security checks (replaces SDLCheck=true from the old VS project)
    add_compile_options(/sdl)
endif()

# ---------------------------------------------------------------------------
# SQLite3 - bundled amalgamation built as its own static library
# ---------------------------------------------------------------------------
add_library(sqlite3 STATIC
    src/sqlite3.c
)

target_include_directories(sqlite3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Unicode CRT / Windows API (replaces CharacterSet=Unicode from the old VS project)
target_compile_definitions(sqlite3 PRIVATE UNICODE _UNICODE)

# ---------------------------------------------------------------------------
# ClipThat executable
# ---------------------------------------------------------------------------
# WIN32 = Windows GUI subsystem with the WinMain entry point, no console window
add_executable(ClipThat WIN32
    src/clipboard.c
    src/database.c
    src/history.c
    src/historyView.c
    src/historyWindow.c
    src/main.c
    src/mainWindow.c
    src/paste.c
    src/path.c
    src/session.c
    src/startup.c
    src/trayIcon.c
    resources/ClipThat.rc
)

target_include_directories(ClipThat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(ClipThat PRIVATE UNICODE _UNICODE)

target_link_libraries(ClipThat PRIVATE
    sqlite3   # bundled static library built from src/sqlite3.c
    kernel32
    user32    # windows, clipboard, hotkeys, simulated input
    comctl32  # SetWindowSubclass (search box + listbox keyboard flow)
    advapi32  # registry API (startup registration)
    shell32   # tray icon, SHGetKnownFolderPath
    ole32     # CoTaskMemFree
)
