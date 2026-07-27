#include "Common.h"
#include "CustomizationUI.h"
#include "Config.h"
#include "EmbeddedCharacterMesh.h"
#include "Application.h"

namespace Visuals3DApp
{
#include "Modules/AppState.inc"
#include "Modules/UIWidgets.inc"
#include "Modules/DirectXRenderer.inc"
#include "Modules/ConfigUI.inc"
#include "Modules/SoundWalkUI.inc"
#include "Modules/VisualControlsUI.inc"
#include "Modules/VisualRenderer.inc"
#include "Modules/MainRender.inc"
#include "Modules/Application.inc"

    int Run(HINSTANCE instance)
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
        windowClass.lpszClassName = L"DarkVisuals3DWindow";

        if (!RegisterClassExW(&windowClass))
            return 0;

        RECT workArea{};
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        const int x = workArea.left + ((workArea.right - workArea.left) - WINDOW_W) / 2;
        const int y = workArea.top + ((workArea.bottom - workArea.top) - WINDOW_H) / 2;

        HWND hwnd = CreateWindowExW(
            WS_EX_APPWINDOW | WS_EX_LAYERED,
            windowClass.lpszClassName,
            L"Visuals 3D",
            WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
            x, y, WINDOW_W, WINDOW_H,
            nullptr, nullptr, instance, nullptr);

        if (!hwnd)
            return 0;

        // Keep the main window at 90% opacity.
        SetLayeredWindowAttributes(hwnd, 0, static_cast<BYTE>(255 * 0.90f), LWA_ALPHA);

        HRGN region = CreateRoundRectRgn(0, 0, WINDOW_W + 1, WINDOW_H + 1, 12, 12);
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
}
