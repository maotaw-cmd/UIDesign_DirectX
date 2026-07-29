#include "../Core/App.h"
#include "../User/UserLogic.h"

bool App::Initialize(HINSTANCE instance)
{
    SetProcessDPIAware();

    if (FAILED(CreateFactories()))
        return false;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    // The window has a fixed client size. Avoid forced full redraws caused by
    // CS_HREDRAW/CS_VREDRAW during shell move operations.
    windowClass.style = 0;
    windowClass.lpfnWndProc = StaticWindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.lpszClassName = L"MaotawUIWindow";

    if (!RegisterClassExW(&windowClass))
        return false;

    const int x = (GetSystemMetrics(SM_CXSCREEN) - Layout::Width) / 2;
    const int y = (GetSystemMetrics(SM_CYSCREEN) - Layout::Height) / 2;

    hwnd_ = CreateWindowExW(
        // WS_EX_APPWINDOW forces this borderless window to appear in the
        // Windows taskbar. WS_EX_TOOLWINDOW intentionally hides it there.
        WS_EX_LAYERED | WS_EX_APPWINDOW,
        windowClass.lpszClassName,
        L"MaotawUI",
        WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
        x,
        y,
        Layout::Width,
        Layout::Height,
        nullptr,
        nullptr,
        instance,
        this);

    if (!hwnd_)
        return false;

    // Uniform transparency keeps the whole window dark and glass-like.
    SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);

    HRGN region = CreateRoundRectRgn(0, 0, Layout::Width + 1, Layout::Height + 1, 12, 12);
    SetWindowRgn(hwnd_, region, TRUE);

    RefreshConfigs();
    UserLogic::OnStart(*this);

    // Give the shell a real taskbar entry and make taskbar clicks restore and
    // activate the window after it has been minimized or covered.
    ShowWindow(hwnd_, SW_SHOWNORMAL);
    UpdateWindow(hwnd_);
    SetTimer(hwnd_, 1, 33, nullptr);
    return true;
}

int App::Run()
{
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

void App::Shutdown()
{
    if (hwnd_)
        KillTimer(hwnd_, 1);

    UserLogic::OnShutdown(*this);

    DiscardDeviceResources();
    SafeRelease(icon_);
    SafeRelease(damageText_);
    SafeRelease(tiny_);
    SafeRelease(small_);
    SafeRelease(text_);
    SafeRelease(writeFactory_);
    SafeRelease(d2dFactory_);
}
