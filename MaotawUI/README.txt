MaotawUI - Modular Visual Studio 2022 Project
==============================================

Open MaotawUI.sln, select x64, then build Debug or Release.
No external libraries are required. Direct2D and DirectWrite are linked by the project.

PROJECT MAP
-----------
MaotawUI/Core/Types.h
  Layout constants, theme colours, glyphs, Settings, and VisualProfile.

MaotawUI/Core/App.h
  Main App class and stable public extension accessors.

MaotawUI/App/Application.cpp
  Startup, window creation, message loop, and shutdown.

MaotawUI/App/WindowProc.cpp
  Mouse, keyboard, timer, drag, wheel, and window messages.

MaotawUI/App/Render.cpp
  Frame order and top-level page selection.

MaotawUI/Rendering/Renderer.cpp
  Direct2D/DirectWrite resources and primitive drawing functions.

MaotawUI/UI/Controls.cpp
  Toggles, sliders, dropdowns, colour picker, text input, and buttons.

MaotawUI/Features/ConfigSystem.cpp
  Save/load/delete config files and the draggable config popup.

MaotawUI/Pages/VisualsPage.cpp
  Visual options, 3D character preview, ESP box/bars/glow, and sidebar.

MaotawUI/Pages/SettingsPage.cpp
  Window, outline, accent, and particle customisation.

MaotawUI/Pages/ProfilePage.cpp
  Full profile/config management page.

MaotawUI/User/UserLogic.cpp
  Safe hooks for custom startup, frame, render, and shutdown logic.
  Most users should begin here.

MaotawUI/User/CustomPage.cpp
  Isolated member-function page where built-in UI helpers can be used directly.
  Add your own toggles, sliders, dropdowns, colours, and logic here.

EASY CUSTOM LOGIC
-----------------
1. Open User/UserLogic.cpp.
2. Put one-time setup in OnStart.
3. Put lightweight repeated logic in OnFrame.
4. Put cleanup in OnShutdown.
5. Access settings through app.MutableSettings().
6. Access the native window through app.WindowHandle().

EASY CUSTOM UI
--------------
1. Add your values to Settings in Core/Types.h, or add private state to App in Core/App.h.
2. Open User/CustomPage.cpp.
3. Use ToggleRow, SliderControl, ComboControl, ColorRow, ActionButton, and DrawText.
4. Keep feature logic in User/UserLogic.cpp or a new file under User/.

ASSETS
------
Assets/character.glb and Assets/Textures are copied next to the built executable.
CharacterPreviewMesh.h is the baked dependency-free preview mesh used by the UI.

CONFIGS
-------
Saved to %APPDATA%\MaotawUI\Configs.
The complete Settings structure is saved and restored.


WINDOW MOVE OPTIMISATION
------------------------
During a native window drag, MaotawUI pauses its 60 FPS animation timer and lets Windows move the last rendered surface. Rendering resumes immediately when the drag ends. This prevents the textured 3D preview and particle system from causing stutter while moving the window.

PAGE ROUTING
------------
Built-in sidebar pages are routed in User/CustomPage.cpp.
Page 0 is Visuals, page 1 is Settings, and page 3 is Profile/configs.
When adding a sidebar page, add its case to DrawOtherPage().


LOCAL / ENEMY PROFILE BEHAVIOUR
--------------------------------
- Local and Enemy begin with the exact same canonical defaults.
- They are independent after startup; editing one never edits the other.
- Switching the selector never resets or copies profile values.
- Config files store both profiles. Config format version is 8.
- Older binary configs are intentionally rejected to avoid loading mismatched struct layouts as random options.


PERFORMANCE NOTES
-----------------
- Animation is capped at about 30 FPS to reduce CPU/GPU usage while remaining smooth.
- Static pages redraw only after input or state changes.
- Native window dragging pauses scene animation and moves the cached frame.
- The 3D preview reuses triangle storage and rejects hidden faces using GLB normals.
- Glow uses three outline layers instead of four.
