VISUALS 3D - CLEAN REBUILD

Open Visuals3D.sln and build x64 / Release.
Required: Visual Studio 2022, Desktop development with C++, Windows 10/11 SDK.

Only these compile:
  src/main.cpp
  src/Application.cpp
  src/Config.cpp

The files in src/Modules are editable feature modules included exactly once by Application.cpp.
Do not mark any .inc file as C/C++ source. This prevents duplicate functions and ambiguous overloads.

Beginner editing:
  src/CustomizationUI.h                  colours and defaults
  src/Modules/VisualControlsUI.inc       menu controls
  src/Modules/VisualRenderer.inc         boxes, health bars and preview overlays
  src/Modules/SoundWalkUI.inc            sound walk
  src/Modules/ConfigUI.inc               config panel
  src/Modules/UIWidgets.inc              reusable controls
  src/Modules/DirectXRenderer.inc        DirectX and 3D model
