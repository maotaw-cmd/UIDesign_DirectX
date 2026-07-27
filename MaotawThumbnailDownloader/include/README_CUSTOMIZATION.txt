EDITING MAP
===========

src/App.cpp
- YouTube link parsing and thumbnail download
- Save folder and persistent library
- Settings values and cleanup logic
- Shared application state

src/Draw.cpp
- Every Direct2D drawing function
- Home, Library, Favorites and Settings pages
- Cards, buttons, icons, previews and scrollbar

src/Window.cpp
- Borderless Win32 window
- Mouse, keyboard, scrolling and hit testing
- Window procedure and application startup

src/main.cpp
- Real Windows entry point
- Includes the three modules in the correct order

IMPORTANT
=========
The project uses a unity build intentionally. Visual Studio compiles only main.cpp.
App.cpp, Draw.cpp and Window.cpp remain separate and editable but are included by
main.cpp. Do not enable "Compile" for the three module files or duplicate symbol
errors will occur.
