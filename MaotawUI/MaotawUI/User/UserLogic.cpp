#include "UserLogic.h"
#include "../Core/App.h"

namespace UserLogic
{
    void OnStart(App& app)
    {
        // Called once after the window and UI are ready.
        // Example: HWND hwnd = app.WindowHandle();
        (void)app;
    }

    void OnFrame(App& app)
    {
        // Called by the existing 60 FPS timer. Put lightweight update logic here.
        // Example: app.MutableSettings().enemy.glow = true;
        (void)app;
    }

    void OnRender(App& app)
    {
        // Called after MaotawUI draws its UI and before the frame is presented.
        // Keep custom Direct2D/UI drawing isolated here or add a dedicated module.
        (void)app;
    }

    void OnShutdown(App& app)
    {
        // Called once before Direct2D and window resources are released.
        (void)app;
    }
}
