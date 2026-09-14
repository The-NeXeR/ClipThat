# ClipThat

A lightweight clipboard manager for Windows, written in C with Win32 and SQLite3.

ClipThat runs in the system tray and saves your clipboard history so you can access old entries whenever you need them.

## Features

- Clipboard history
- Duplicate detection
- Most Recently Used (MRU) ordering
- Configurable history limit
- System tray support
- Global hotkey: `Ctrl + Alt + H`
- Double-click an entry to restore and paste it
- SQLite persistence
- UTF-16 clipboard support
- No external runtime dependencies

## Current Version

**v1.0.0**

## How It Works

ClipThat uses the Windows Clipboard Listener API to detect clipboard changes.

While the program is running, clipboard entries are kept in memory using a linked list. The history is displayed through the history window.

When ClipThat closes, the current history is saved to the SQLite database. When the program starts again, the saved history is loaded back into memory.

## Database

The database is created automatically when ClipThat starts.

It is stored in:

```text
%LOCALAPPDATA%\ClipThat\
```

The database file is:

```text
clipThat.db
```

## Hotkey Usage

You can press:

```text
Ctrl + Alt + H
```

to open the clipboard history.

You can also right-click the ClipThat icon in the system tray. The menu lets you open the history or exit the application.

## Building

### Requirements

- Windows
- Visual Studio
- Desktop development with C++ workload
- Windows SDK

Even though ClipThat is written in C, the **Desktop development with C++** workload is required because it includes the Windows development tools and SDK needed to build the project.

## License

This project is licensed under the MIT License.

SQLite is separately released into the public domain.
