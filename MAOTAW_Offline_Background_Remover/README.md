# MAOTAW Offline Background Remover — Modular Project

This version is split into smaller files so it is easier to edit.

## File structure
- `src/main.cpp` — app startup and window creation
- `src/App.h` — shared includes, globals, and declarations
- `src/AppGlobals.cpp` — global variables
- `src/Utilities.cpp` — helpers, paths, logging, process launching, downloads
- `src/Runtime.cpp` — Python runtime setup and background-removal worker script
- `src/Graphics.cpp` — Direct2D creation, icons, text, gradient, drawing helpers
- `src/Layout.cpp` — UI rectangles and layout positions
- `src/BitmapEditing.cpp` — output bitmap loading/saving and feather/refine helpers
- `src/UI.cpp` — all visible UI drawing and app actions
- `src/Window.cpp` — window procedure and mouse/keyboard message handling

## Build
Open `MAOTAWBackgroundRemover.sln` in Visual Studio 2022, choose **x64**, then build.

## Notes
This is the same app logic as the previous ZIP, but organized into multiple source files so it is easier to customize.

## Formatting
The source files use normal multi-line formatting. A `.clang-format` file is included for Visual Studio Format Document.
