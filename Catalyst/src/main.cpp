// Beginner-friendly project entry point.
// Edit visual defaults in Core/ProjectCore.hpp.
// Edit Sound Walk popup controls in UI/SoundWalkSettings.hpp.
// Edit other controls and preview drawing in UI/VisualsUI.hpp.
// Edit config persistence in Config/ConfigManager.hpp.
// Edit the config page in Config/ConfigUI.hpp.
// Edit DirectX/shaders in Renderer/DirectXRenderer.hpp.

#include "Core/ProjectCore.hpp"
#include "Config/ConfigManager.hpp"
#include "Renderer/DirectXRenderer.hpp"
#include "Config/ConfigUI.hpp"
#include "UI/SoundWalkSettings.hpp"
#include "UI/VisualsUI.hpp"
#include "App/Application.hpp"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.lpszClassName = L"CatalystWindow";

    if (!RegisterClassExW(&windowClass))
        return 0;

    RECT workArea{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);

    const int x =
        workArea.left +
        ((workArea.right - workArea.left) - WINDOW_W) / 2;

    const int y =
        workArea.top +
        ((workArea.bottom - workArea.top) - WINDOW_H) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        windowClass.lpszClassName,
        L"Catalyst",
        WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
        x,
        y,
        WINDOW_W,
        WINDOW_H,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!hwnd)
        return 0;

    HRGN region = CreateRoundRectRgn(
        0, 0,
        WINDOW_W + 1,
        WINDOW_H + 1,
        12, 12);

    SetWindowRgn(hwnd, region, TRUE);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
