#include "App.h"

int WINAPI wWinMain(HINSTANCE i, HINSTANCE, LPWSTR, int s)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_d2d);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_dw));
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_wic));

    g_dw->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14,
        L"en-us",
        &g_font);
    g_font->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = Proc;
    wc.hInstance = i;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = static_cast<HICON>(LoadImageW(i, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR));
    wc.hIconSm = static_cast<HICON>(LoadImageW(i, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
    wc.lpszClassName = L"MAOTAWBackgroundRemover";
    RegisterClassExW(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"MAOTAW Background Remover",
        WS_POPUP | WS_THICKFRAME,
        (sw - WINDOW_WIDTH) / 2,
        (sh - WINDOW_HEIGHT) / 2,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        nullptr,
        nullptr,
        i,
        nullptr);

    SendMessageW(g_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(wc.hIcon));
    SendMessageW(g_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(wc.hIconSm));

    BOOL dark = TRUE;
    DwmSetWindowAttribute(g_hwnd, 20, &dark, sizeof(dark));

    COLORREF border = RGB(25, 29, 34);
    DwmSetWindowAttribute(g_hwnd, 34, &border, sizeof(border));

    ShowWindow(g_hwnd, s);
    UpdateWindow(g_hwnd);
    SetTimer(g_hwnd, 1, 16, nullptr);

    std::thread([]
    {
        PrepareRuntime();
        Refresh();
    }).detach();

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Release(&g_outputBitmap);
    Release(&g_inputBitmap);
    Release(&g_gradient);
    Release(&g_stops);
    Release(&g_brush);
    Release(&g_rt);
    Release(&g_font);
    Release(&g_dw);
    Release(&g_wic);
    Release(&g_d2d);

    CoUninitialize();
    return 0;
}
