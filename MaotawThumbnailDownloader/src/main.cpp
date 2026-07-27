// Maotaw Thumbnail Downloader - Visual Studio unity entry point.
#include "../include/Window.h"
//
// The implementation is separated into editable files:
//   App.cpp    - application state, downloads, storage and settings
//   Draw.cpp   - Direct2D drawing, pages, cards, icons and previews
//   Window.cpp - Win32 window procedure, input and message loop
//
// They are included here in order so all existing internal/static helpers remain
// private while the project still compiles as one reliable translation unit.
#include "App.cpp"
#include "Draw.cpp"
#include "Window.cpp"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    return RunApplication(instance, showCommand);
}
