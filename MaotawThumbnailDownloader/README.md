# Maotaw Thumbnail Downloader

Visual Studio-ready C++ project using Win32, Direct2D, DirectWrite and WIC.

## Open and build
1. Open `MaotawThumbnailDownloader.sln`.
2. Select `x64` and `Debug` or `Release`.
3. Press `F5` or use **Build > Build Solution**.

No external packages are required.

## Project layout
- `src/main.cpp` — small application entry point
- `src/App.cpp` + `include/App.h` — downloads, state, files and settings
- `src/Draw.cpp` + `include/Draw.h` — all Direct2D rendering
- `src/Window.cpp` + `include/Window.h` — Win32 window and input
- `include/AppConfig.h` — easy sizes and layout values
- `include/Models.h` — shared Library item model

The project uses a unity build through `main.cpp`: App.cpp, Draw.cpp and Window.cpp remain visible and editable in Visual Studio, but only `main.cpp` is compiled directly. This avoids duplicate symbol and missing internal-helper errors while keeping the code separated by purpose.
