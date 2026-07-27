#pragma once

#include <windows.h>

// Creates the borderless Win32 window, processes input/messages,
// initializes Direct2D/DirectWrite/WIC and runs the message loop.
int RunApplication(HINSTANCE instance, int showCommand);
