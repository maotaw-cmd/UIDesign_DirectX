#include "Core/App.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    App app;
    if (!app.Initialize(instance))
    {
        MessageBoxW(nullptr, L"Failed to initialize Direct2D.", L"Error", MB_OK | MB_ICONERROR);
        app.Shutdown();
        return 1;
    }

    const int result = app.Run();
    app.Shutdown();
    return result;
}
