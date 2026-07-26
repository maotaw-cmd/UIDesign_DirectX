// ============================================================================
// Dark Direct2D menu mockup - single-file C++17 / Win32 / Direct2D
// Dark transparent full-width Direct2D menu UI with independent Aimbot/Visuals controls.
//
// Build (Visual Studio x64, Unicode, Windows subsystem):
//   d2d1.lib
//   dwrite.lib
//
// Notes:
// - This is a UI template only.
// - Drag the top bar to move the window.
// - Esc closes the window. Backspace clears a keybind while capturing.
// - The old left navbar and version label were removed.
// - UI demonstration only: no game process, memory, aiming, or overlay integration.
// - Visual preview removed; top save/settings buttons use large reliable hit areas.
// ============================================================================

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include "PreviewRenderer.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

template <typename T>
static T MaxValue(T a, T b)
{
    return (a > b) ? a : b;
}

template <typename T>
static T MinValue(T a, T b)
{
    return (a < b) ? a : b;
}

template <typename T>
static void SafeRelease(T*& value)
{
    if (value)
    {
        value->Release();
        value = nullptr;
    }
}

static float Clamp(float value, float minimum, float maximum)
{
    return MaxValue(minimum, MinValue(value, maximum));
}

static bool Hit(float x, float y, float left, float top, float right, float bottom)
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

static D2D1_COLOR_F MakeColor(std::uint32_t rgb, float alpha = 1.0f)
{
    return D2D1::ColorF(
        static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
        static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
        static_cast<float>(rgb & 0xFF) / 255.0f,
        alpha);
}

static D2D1_COLOR_F WithAlpha(const D2D1_COLOR_F& color, float alpha)
{
    return D2D1::ColorF(color.r, color.g, color.b, alpha);
}

static D2D1_COLOR_F BlendColor(
    const D2D1_COLOR_F& base,
    const D2D1_COLOR_F& tint,
    float amount)
{
    amount = Clamp(amount, 0.0f, 1.0f);
    const float inverse = 1.0f - amount;

    return D2D1::ColorF(
        base.r * inverse + tint.r * amount,
        base.g * inverse + tint.g * amount,
        base.b * inverse + tint.b * amount,
        1.0f);
}

static D2D1_COLOR_F HsvToColor(float hue, float saturation, float value)
{
    hue = hue - std::floor(hue);
    saturation = Clamp(saturation, 0.0f, 1.0f);
    value = Clamp(value, 0.0f, 1.0f);

    const float scaled = hue * 6.0f;
    const int sector = static_cast<int>(std::floor(scaled)) % 6;
    const float fraction = scaled - std::floor(scaled);
    const float p = value * (1.0f - saturation);
    const float q = value * (1.0f - fraction * saturation);
    const float t = value * (1.0f - (1.0f - fraction) * saturation);

    switch (sector)
    {
    case 0: return D2D1::ColorF(value, t, p, 1.0f);
    case 1: return D2D1::ColorF(q, value, p, 1.0f);
    case 2: return D2D1::ColorF(p, value, t, 1.0f);
    case 3: return D2D1::ColorF(p, q, value, 1.0f);
    case 4: return D2D1::ColorF(t, p, value, 1.0f);
    default: return D2D1::ColorF(value, p, q, 1.0f);
    }
}

static void ColorToHsv(const D2D1_COLOR_F& color, float& hue, float& saturation, float& value)
{
    const float maximum = MaxValue(color.r, MaxValue(color.g, color.b));
    const float minimum = MinValue(color.r, MinValue(color.g, color.b));
    const float delta = maximum - minimum;

    value = maximum;
    saturation = maximum <= 0.00001f ? 0.0f : delta / maximum;

    if (delta <= 0.00001f)
    {
        hue = 0.0f;
        return;
    }

    if (maximum == color.r)
        hue = std::fmod((color.g - color.b) / delta, 6.0f) / 6.0f;
    else if (maximum == color.g)
        hue = (((color.b - color.r) / delta) + 2.0f) / 6.0f;
    else
        hue = (((color.r - color.g) / delta) + 4.0f) / 6.0f;

    if (hue < 0.0f)
        hue += 1.0f;
}

namespace UI
{
    constexpr int Width = 796;
    constexpr int Height = 470;
    constexpr int SideWidth = 260;
    constexpr int WindowGap = 10;

    constexpr float SidebarWidth = 64.0f;
    constexpr float TopBarHeight = 0.0f;
    constexpr float ContentPadding = 18.0f;
    constexpr float DragHeight = 34.0f;
}

namespace Theme
{
    const D2D1_COLOR_F Window = MakeColor(0x010203);
    const D2D1_COLOR_F TopBar = MakeColor(0x020304);
    const D2D1_COLOR_F Sidebar = MakeColor(0x08090B);
    const D2D1_COLOR_F SidebarBorder = MakeColor(0x15171A);

    const D2D1_COLOR_F Card = MakeColor(0x07090C, 0.96f);
    const D2D1_COLOR_F CardBorder = MakeColor(0x111317);
    const D2D1_COLOR_F CardInner = MakeColor(0x0D0F12);
    const D2D1_COLOR_F RowHover = MakeColor(0x111418, 0.72f);
    const D2D1_COLOR_F Divider = MakeColor(0x17191D, 0.8f);

    const D2D1_COLOR_F Text = MakeColor(0xE6E6E6);
    const D2D1_COLOR_F Muted = MakeColor(0x7D8189);
    const D2D1_COLOR_F Muted2 = MakeColor(0x5E636A);

    const D2D1_COLOR_F Accent = MakeColor(0xB31622);
    const D2D1_COLOR_F AccentSoft = MakeColor(0x63141B, 0.42f);
    const D2D1_COLOR_F AccentBright = MakeColor(0xD62936);
    const D2D1_COLOR_F SliderKnob = MakeColor(0x393939);
    const D2D1_COLOR_F Input = MakeColor(0x090B0E);
    const D2D1_COLOR_F InputHover = MakeColor(0x101318);
}

namespace Icon
{
    constexpr const wchar_t* Search = L"\xE721";
    constexpr const wchar_t* Gear = L"\xE713";
    constexpr const wchar_t* Mouse = L"\xE962";
    constexpr const wchar_t* Skull = L"\xE9A1";
    constexpr const wchar_t* Anti = L"\xE72D";
    constexpr const wchar_t* Eye = L"\xE890";
    constexpr const wchar_t* Menu = L"\xE700";
    constexpr const wchar_t* People = L"\xE716";
    constexpr const wchar_t* Phone = L"\xE717";
    constexpr const wchar_t* Code = L"\xE943";
    constexpr const wchar_t* Folder = L"\xE8B7";
    constexpr const wchar_t* Save = L"\xE74E";
    constexpr const wchar_t* Load = L"\xE72C";
    constexpr const wchar_t* Delete = L"\xE74D";
    constexpr const wchar_t* Add = L"\xE710";
    constexpr const wchar_t* File = L"\xE8A5";
    constexpr const wchar_t* Check = L"\xE73E";
    constexpr const wchar_t* Chevron = L"\xE70D";
    constexpr const wchar_t* Help = L"\xE897";
    constexpr const wchar_t* Drop = L"\xE790";
}

constexpr int HotkeyKeyMask = 0x00FF;
constexpr int HotkeyCtrlFlag = 0x0100;
constexpr int HotkeyAltFlag = 0x0200;
constexpr int HotkeyShiftFlag = 0x0400;

struct NavItem
{
    std::wstring label;
    const wchar_t* icon;
};

static LRESULT CALLBACK EmptySideWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    bool handled = false;
    const LRESULT result = PreviewRenderer::HandleMessage(hwnd, message, wParam, lParam, handled);
    if (handled)
        return result;

    if (message == WM_NCHITTEST)
        return HTCAPTION;

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

class App
{
public:
    bool Initialize(HINSTANCE instance);
    int Run();
    void Shutdown();

private:

    struct DropdownOverlay
    {
        bool valid = false;
        float buttonLeft = 0.0f;
        float buttonTop = 0.0f;
        float buttonRight = 0.0f;
        float buttonBottom = 0.0f;
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float itemHeight = 28.0f;
        std::vector<std::wstring> items;
        int* selected = nullptr;
    };

    HWND hwnd_ = nullptr;
    HWND sideHwnd_ = nullptr;
    ID2D1Factory* d2dFactory_ = nullptr;
    ID2D1HwndRenderTarget* target_ = nullptr;
    ID2D1SolidColorBrush* brush_ = nullptr;
    IDWriteFactory* writeFactory_ = nullptr;
    IDWriteTextFormat* font_ = nullptr;
    IDWriteTextFormat* smallFont_ = nullptr;
    IDWriteTextFormat* titleFont_ = nullptr;
    IDWriteTextFormat* logoFont_ = nullptr;
    IDWriteTextFormat* iconFont_ = nullptr;

    POINT mouse_{ -1000, -1000 };
    bool mouseDown_ = false;
    bool mousePressed_ = false;
    // The top-most popup owns the current click. Background controls must ignore it.
    bool popupOwnsClick_ = false;
    bool drawingVisualPopup_ = false;

    int activeAimbotPopup_ = 0;
    float aimbotPopupAnim_ = 0.0f;
    D2D1_RECT_F aimbotPopupBounds_{};
    bool aimbotPopupBoundsValid_ = false;

    // Aimbot page values. These are UI settings only.
    bool aimEnabled_ = false;
    bool silentAim_ = false;
    bool teamCheck_ = true;
    bool visibleOnly_ = true;
    bool prediction_ = false;
    float aimFov_ = 18.0f;
    float aimSmooth_ = 6.0f;
    float aimDistance_ = 250.0f;
    int targetBone_ = 0;

    // Extra UI-only controls shown inside selected Aimbot popups.
    int aimPriority_ = 0;          // 0 closest, 1 lowest health, 2 field of view
    bool aimAutoSwitch_ = true;
    bool aimIgnoreDowned_ = true;
    float aimStartDelay_ = 35.0f;

    int silentMode_ = 1;           // 0 subtle, 1 balanced, 2 aggressive
    bool silentVisibleFallback_ = true;
    bool silentThroughSmoke_ = false;
    float silentHitChance_ = 72.0f;

    int predictionMode_ = 1;       // 0 linear, 1 adaptive, 2 advanced
    bool predictionGravity_ = true;
    bool predictionAirborne_ = true;
    float predictionStrength_ = 1.0f;

    // Independent visual options. Enabling one never changes another.
    bool espBox_ = true;
    bool espCornerBox_ = false;
    bool espHealthBar_ = true;
    bool espName_ = true;
    bool espDistance_ = false;
    bool espSnapline_ = false;
    bool espSkeleton_ = false;
    bool espWeapon_ = false;
    bool espFilled_ = true;
    bool espDamage_ = true;
    bool visualGlow_ = true;
    float visualGlowThickness_ = 0.7f;
    int boxStyle_ = 2;       // 0 off, 1 box, 2 cornered, 3 box+filled, 4 cornered+filled
    int healthStyle_ = 1;    // 0 normal, 1 segmented
    float healthBarWidth_ = 6.0f;
    int activeVisualPopup_ = 0; // 0 none, 1 box, 2 health, 3 name, 4 distance, 5 weapon, 6 snapline, 7 damage, 8 glow
    float visualPopupAnim_ = 0.0f;
    D2D1_RECT_F visualPopupBounds_{};
    bool visualPopupBoundsValid_ = false;
    bool windowsVisible_ = true;
    bool previewVisible_ = true;

    D2D1_COLOR_F boxColor_ = MakeColor(0xFFFFFF);
    D2D1_COLOR_F filledColor_ = MakeColor(0xB31622);
    D2D1_COLOR_F healthColor_ = MakeColor(0x42D66B);
    D2D1_COLOR_F healthBackColor_ = MakeColor(0xB31622);
    D2D1_COLOR_F damageColor_ = MakeColor(0xFF4050);
    D2D1_COLOR_F visualGlowColor_ = MakeColor(0xB31622);
    D2D1_COLOR_F lineColor_ = MakeColor(0xFFFFFF);
    D2D1_COLOR_F skeletonColor_ = MakeColor(0xD5DBFF);

    bool dropdownOpen_ = false;
    DropdownOverlay dropdownOverlay_{};
    int activeSlider_ = -1;

    bool settingsOpen_ = false;
    bool configOpen_ = false;
    D2D1_RECT_F configPopupBounds_{};
    bool configPopupBoundsValid_ = false;
    bool particlesEnabled_ = true;
    float particleSpeed_ = 22.0f;
    float particleAmount_ = 34.0f;
    float previewScale_ = 1.00f;
    float windowOpacity_ = 222.0f; // affects only the main and preview HWNDs
    float popupOpacity_ = 250.0f;  // affects only floating popup surfaces
    float settingsScroll_ = 0.0f;
    int openCloseHotkey_ = VK_F11;
    int exitHotkey_ = VK_END;

    std::wstring configName_;
    bool configNameFocused_ = false;
    std::vector<std::wstring> configs_;
    int selectedConfig_ = -1;
    float configScroll_ = 0.0f;
    std::wstring configStatus_;
    bool configStatusOk_ = true;
    float configStatusTime_ = 0.0f;

    // Window tint and UI accent are independent color settings.
    D2D1_COLOR_F backgroundColor_ = MakeColor(0xFFFFFF);
    D2D1_COLOR_F accentColor_ = MakeColor(0xB31622);

    int aimbotKeybind_ = 0;
    bool keybindCapture_ = false;
    int* keybindCaptureTarget_ = nullptr;
    bool hotkeyRegistered_ = false;

    bool colorPickerOpen_ = false;
    D2D1_COLOR_F* pickerTarget_ = nullptr;
    D2D1_COLOR_F pickerOriginal_{};
    D2D1_COLOR_F pickerWorking_{};
    float pickerHue_ = 0.0f;
    float pickerSaturation_ = 0.0f;
    float pickerValue_ = 1.0f;
    int activeColorArea_ = -1; // 0 = saturation/value, 1 = hue


    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT CreateFactories();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    void Render();
    void DrawShell(float width, float height);
    void DrawGlassWindowOutline(float width, float height);
    void DrawSidebar(float height);
    void DrawPreviewToggle(float width);
    void DrawCards();
    void DrawSettingsPage();
    void DrawConfigPage();
    void DrawParticles(float width, float height);

    void DrawAimbotCard(float x, float y, float w, float h);
    void DrawAimbotFeatureRow(const std::wstring& label, bool& value, int popupId, float x, float y, float w);
    void DrawAimbotPopup(float cardX, float cardY, float cardW);
    void DrawVisualsCard(float x, float y, float w, float h);
    void DrawVisualFeatureRow(const std::wstring& label, bool& value, int popupId, float x, float y, float w);
    void DrawVisualFeaturePopup(float cardX, float cardY, float cardW);

    void SetBrush(D2D1_COLOR_F color);
    void FillRect(float left, float top, float right, float bottom, D2D1_COLOR_F color);
    void DrawRect(float left, float top, float right, float bottom, D2D1_COLOR_F color, float stroke = 1.0f);
    void FillRound(float left, float top, float right, float bottom, float radius, D2D1_COLOR_F color);
    void DrawRound(float left, float top, float right, float bottom, float radius, D2D1_COLOR_F color, float stroke = 1.0f);
    void DrawLine(float x1, float y1, float x2, float y2, D2D1_COLOR_F color, float stroke = 1.0f);
    void DrawTextLine(const std::wstring& text, float left, float top, float right, float bottom,
        D2D1_COLOR_F color, IDWriteTextFormat* format = nullptr,
        DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING);
    void DrawIcon(const wchar_t* icon, float left, float top, float right, float bottom, D2D1_COLOR_F color);
    void DrawTriangleLogo(float x, float y);

    bool Hover(float left, float top, float right, float bottom) const;
    D2D1_COLOR_F AccentBright() const;
    D2D1_COLOR_F AccentSoft(float alpha = 0.22f) const;
    D2D1_COLOR_F PopupColor(const D2D1_COLOR_F& color) const;
    void Card(float x, float y, float w, float h, const std::wstring& title);
    void Divider(float x, float y, float w);
    void Toggle(const std::wstring& label, bool& value, float x, float y, float w);
    void Slider(const std::wstring& label, float& value, float minimum, float maximum,
        bool isFloat, float x, float y, float w, int sliderId);
    void Dropdown(const std::wstring& label, const std::vector<std::wstring>& items, int& selected, float x, float y, float w);
    void DrawDropdownOverlay();
    void ColorRow(const std::wstring& label, D2D1_COLOR_F& color, float x, float y, float w, bool second = false);
    void KeybindControl(const std::wstring& label, int& encodedKey, float x, float y, float w);
    bool Button(const std::wstring& label, const wchar_t* icon, float x, float y, float w, float h, bool accent = false, bool danger = false);
    void TextInput(const std::wstring& label, std::wstring& value, bool& focused, float x, float y, float w);
    std::filesystem::path ConfigDirectory() const;
    std::filesystem::path ConfigPath(const std::wstring& name) const;
    std::wstring CleanConfigName(const std::wstring& name) const;
    bool SaveConfig(const std::wstring& name);
    bool LoadConfig(const std::wstring& name);
    bool DeleteConfig(const std::wstring& name);
    void RefreshConfigs();
    void ShowConfigStatus(const std::wstring& text, bool ok);
    std::wstring BaseKeyName(int virtualKey) const;
    std::wstring KeybindName(int encodedKey) const;
    int EncodeHotkey(int virtualKey) const;
    bool IsModifierKey(int virtualKey) const;
    bool MatchesHotkey(int encodedKey, int virtualKey) const;
    void UpdateRegisteredHotkey();
    void OpenColorPicker(D2D1_COLOR_F& color);
    void UpdatePickerColorFromHsv();
    void DrawColorPicker();
    void SyncPreviewSettings();
};

HRESULT App::CreateFactories()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory_);
    if (FAILED(hr))
        return hr;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&writeFactory_));
    if (FAILED(hr))
        return hr;

    auto createFont = [&](const wchar_t* family, float size, DWRITE_FONT_WEIGHT weight, IDWriteTextFormat** out)
        {
            HRESULT result = writeFactory_->CreateTextFormat(
                family,
                nullptr,
                weight,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size,
                L"en-US",
                out);

            if (SUCCEEDED(result))
            {
                (*out)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                (*out)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            }

            return result;
        };

    if (FAILED(createFont(L"Segoe UI", 12.0f, DWRITE_FONT_WEIGHT_NORMAL, &font_)))
        return E_FAIL;
    if (FAILED(createFont(L"Segoe UI", 10.0f, DWRITE_FONT_WEIGHT_NORMAL, &smallFont_)))
        return E_FAIL;
    if (FAILED(createFont(L"Segoe UI", 12.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, &titleFont_)))
        return E_FAIL;
    if (FAILED(createFont(L"Segoe UI", 19.0f, DWRITE_FONT_WEIGHT_BOLD, &logoFont_)))
        return E_FAIL;
    if (FAILED(createFont(L"Segoe MDL2 Assets", 15.0f, DWRITE_FONT_WEIGHT_NORMAL, &iconFont_)))
        return E_FAIL;

    return S_OK;
}

HRESULT App::CreateDeviceResources()
{
    if (target_)
        return S_OK;

    RECT client{};
    GetClientRect(hwnd_, &client);

    HRESULT hr = d2dFactory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_HARDWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        D2D1::HwndRenderTargetProperties(
            hwnd_,
            D2D1::SizeU(client.right - client.left, client.bottom - client.top),
            D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &target_);

    if (FAILED(hr))
        return hr;

    return target_->CreateSolidColorBrush(Theme::Text, &brush_);
}

void App::DiscardDeviceResources()
{
    SafeRelease(brush_);
    SafeRelease(target_);
}

bool App::Initialize(HINSTANCE instance)
{
    SetProcessDPIAware();

    if (FAILED(CreateFactories()))
        return false;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"RenderOneUI_MainWindow";
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;

    if (!RegisterClassExW(&wc))
        return false;

    WNDCLASSEXW sideClass{};
    sideClass.cbSize = sizeof(sideClass);
    sideClass.style = CS_HREDRAW | CS_VREDRAW;
    sideClass.lpfnWndProc = EmptySideWindowProc;
    sideClass.hInstance = instance;
    sideClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    sideClass.hbrBackground = nullptr;
    sideClass.lpszClassName = L"RenderOneUI_PreviewWindow";

    if (!RegisterClassExW(&sideClass))
        return false;

    const int combinedWidth = UI::Width + UI::WindowGap + UI::SideWidth;
    const int x = (GetSystemMetrics(SM_CXSCREEN) - combinedWidth) / 2;
    const int y = (GetSystemMetrics(SM_CYSCREEN) - UI::Height) / 2;

    hwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        wc.lpszClassName,
        L"RenderOneUI",
        WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
        x,
        y,
        UI::Width,
        UI::Height,
        nullptr,
        nullptr,
        instance,
        this);

    if (!hwnd_)
        return false;

    sideHwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        sideClass.lpszClassName,
        L"",
        WS_POPUP,
        x + UI::Width + UI::WindowGap,
        y,
        UI::SideWidth,
        UI::Height,
        hwnd_,
        nullptr,
        instance,
        nullptr);

    if (!sideHwnd_)
        return false;

    SyncPreviewSettings();
    if (!PreviewRenderer::Initialize(sideHwnd_))
    {
        MessageBoxW(hwnd_, L"Could not initialise the 3D character preview.",
            L"Preview error", MB_ICONERROR);
        return false;
    }

    // Both windows must be layered for the alpha value to have any effect.
    const BYTE windowAlpha = static_cast<BYTE>(Clamp(windowOpacity_, 80.0f, 255.0f));
    SetLayeredWindowAttributes(hwnd_, 0, windowAlpha, LWA_ALPHA);
    SetLayeredWindowAttributes(sideHwnd_, 0, windowAlpha, LWA_ALPHA);

    HRGN region = CreateRoundRectRgn(0, 0, UI::Width + 1, UI::Height + 1, 10, 10);
    SetWindowRgn(hwnd_, region, TRUE);
    HRGN sideRegion = CreateRoundRectRgn(0, 0, UI::SideWidth + 1, UI::Height + 1, 10, 10);
    SetWindowRgn(sideHwnd_, sideRegion, TRUE);

    RefreshConfigs();
    configName_ = L"default_config";
    if (std::filesystem::exists(ConfigPath(configName_)))
        LoadConfig(configName_);
    else
        SaveConfig(configName_);
    RefreshConfigs();
    const auto defaultIt = std::find(configs_.begin(), configs_.end(), configName_);
    if (defaultIt != configs_.end())
        selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), defaultIt));

    UpdateRegisteredHotkey();

    ShowWindow(hwnd_, SW_SHOW);
    ShowWindow(sideHwnd_, SW_SHOWNA);
    UpdateWindow(hwnd_);
    UpdateWindow(sideHwnd_);
    SetTimer(hwnd_, 1, 16, nullptr);
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
    {
        KillTimer(hwnd_, 1);
        UnregisterHotKey(hwnd_, 1);
        UnregisterHotKey(hwnd_, 2);
        UnregisterHotKey(hwnd_, 3);
    }

    if (sideHwnd_)
    {
        PreviewRenderer::Shutdown();
        DestroyWindow(sideHwnd_);
        sideHwnd_ = nullptr;
    }

    DiscardDeviceResources();
    SafeRelease(iconFont_);
    SafeRelease(logoFont_);
    SafeRelease(titleFont_);
    SafeRelease(smallFont_);
    SafeRelease(font_);
    SafeRelease(writeFactory_);
    SafeRelease(d2dFactory_);
}

void App::SetBrush(D2D1_COLOR_F color)
{
    brush_->SetColor(color);
}

void App::FillRect(float left, float top, float right, float bottom, D2D1_COLOR_F color)
{
    SetBrush(color);
    target_->FillRectangle(D2D1::RectF(left, top, right, bottom), brush_);
}

void App::DrawRect(float left, float top, float right, float bottom, D2D1_COLOR_F color, float stroke)
{
    SetBrush(color);
    target_->DrawRectangle(D2D1::RectF(left, top, right, bottom), brush_, stroke);
}

void App::FillRound(float left, float top, float right, float bottom, float radius, D2D1_COLOR_F color)
{
    SetBrush(color);
    target_->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radius, radius), brush_);
}

void App::DrawRound(float left, float top, float right, float bottom, float radius, D2D1_COLOR_F color, float stroke)
{
    SetBrush(color);
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radius, radius), brush_, stroke);
}

void App::DrawLine(float x1, float y1, float x2, float y2, D2D1_COLOR_F color, float stroke)
{
    SetBrush(color);
    target_->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), brush_, stroke);
}

void App::DrawTextLine(const std::wstring& text, float left, float top, float right, float bottom,
    D2D1_COLOR_F color, IDWriteTextFormat* format, DWRITE_TEXT_ALIGNMENT alignment)
{
    if (!format)
        format = font_;

    format->SetTextAlignment(alignment);
    SetBrush(color);
    target_->DrawTextW(
        text.c_str(),
        static_cast<UINT32>(text.size()),
        format,
        D2D1::RectF(left, top, right, bottom),
        brush_,
        D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void App::DrawIcon(const wchar_t* icon, float left, float top, float right, float bottom, D2D1_COLOR_F color)
{
    DrawTextLine(icon, left, top, right, bottom, color, iconFont_, DWRITE_TEXT_ALIGNMENT_CENTER);
}

bool App::Hover(float left, float top, float right, float bottom) const
{
    return Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y), left, top, right, bottom);
}

D2D1_COLOR_F App::AccentBright() const
{
    return D2D1::ColorF(
        Clamp(accentColor_.r * 1.22f + 0.03f, 0.0f, 1.0f),
        Clamp(accentColor_.g * 1.22f + 0.03f, 0.0f, 1.0f),
        Clamp(accentColor_.b * 1.22f + 0.03f, 0.0f, 1.0f),
        1.0f);
}

D2D1_COLOR_F App::AccentSoft(float alpha) const
{
    return WithAlpha(accentColor_, alpha);
}

D2D1_COLOR_F App::PopupColor(const D2D1_COLOR_F& color) const
{
    const float opacity = Clamp(popupOpacity_ / 255.0f, 0.20f, 1.0f);
    return D2D1::ColorF(color.r, color.g, color.b, color.a * opacity);
}

void App::DrawTriangleLogo(float x, float y)
{
    const D2D1_COLOR_F white = MakeColor(0xFFFFFF);

    DrawLine(x + 2.0f, y + 21.0f, x + 12.5f, y + 3.0f, white, 1.7f);
    DrawLine(x + 12.5f, y + 3.0f, x + 23.0f, y + 21.0f, white, 1.7f);
    DrawLine(x + 23.0f, y + 21.0f, x + 2.0f, y + 21.0f, white, 1.7f);

    DrawLine(x + 7.0f, y + 18.0f, x + 12.5f, y + 9.0f, white, 1.2f);
    DrawLine(x + 12.5f, y + 9.0f, x + 18.0f, y + 18.0f, white, 1.2f);
    DrawLine(x + 18.0f, y + 18.0f, x + 7.0f, y + 18.0f, white, 1.2f);
}

void App::Card(float x, float y, float w, float h, const std::wstring& title)
{
    // Fully transparent section: no card fill and no card border.
    // Only the section title and controls remain on the window background.
    DrawTextLine(title, x + 4.0f, y + 8.0f, x + w - 4.0f, y + 34.0f, Theme::Muted, titleFont_);
}

void App::Divider(float x, float y, float w)
{
    DrawLine(x, y, x + w, y, Theme::Divider, 1.0f);
}


void App::Toggle(const std::wstring& label, bool& value, float x, float y, float w)
{
    const float rowH = 28.0f;
    const bool hovered = Hover(x, y, x + w, y + rowH);

    DrawTextLine(label, x, y, x + w - 48.0f, y + rowH,
        hovered ? Theme::Text : MakeColor(0xD6D6D6), font_);

    const float trackW = 31.0f;
    const float trackH = 14.0f;
    const float trackX = x + w - trackW;
    const float trackY = y + (rowH - trackH) * 0.5f;

    const D2D1_COLOR_F trackColor = value
        ? WithAlpha(accentColor_, hovered ? 0.58f : 0.44f)
        : (hovered ? MakeColor(0x232832) : MakeColor(0x191D24));

    FillRound(trackX, trackY, trackX + trackW, trackY + trackH, 7.0f, trackColor);
    DrawRound(trackX, trackY, trackX + trackW, trackY + trackH, 7.0f,
        value ? WithAlpha(AccentBright(), 0.42f) : MakeColor(0x252A33), 1.0f);

    const float knobSize = 12.0f;
    const float knobX = value ? trackX + trackW - knobSize - 1.0f : trackX + 1.0f;
    const float knobY = trackY + 1.0f;

    FillRound(knobX, knobY, knobX + knobSize, knobY + knobSize, 6.0f,
        value ? AccentBright() : MakeColor(0x343A45));

    if (value)
        DrawRound(knobX, knobY, knobX + knobSize, knobY + knobSize, 6.0f,
            WithAlpha(MakeColor(0xFFFFFF), 0.15f), 1.0f);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && hovered && mousePressed_)
        value = !value;
}

void App::Slider(const std::wstring& label, float& value, float minimum, float maximum,
    bool isFloat, float x, float y, float w, int sliderId)
{
    DrawTextLine(label, x, y, x + 126.0f, y + 24.0f, Theme::Text, font_);

    wchar_t buffer[64]{};
    if (isFloat)
        swprintf_s(buffer, L"%.1f", value);
    else
        swprintf_s(buffer, L"%.0f", value);

    DrawTextLine(buffer, x + 126.0f, y, x + 170.0f, y + 24.0f,
        Theme::Text, font_, DWRITE_TEXT_ALIGNMENT_TRAILING);

    const float trackX = x + 181.0f;
    const float trackY = y + 14.0f;
    const float trackW = MaxValue(30.0f, w - 191.0f);
    const float ratio = Clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
    const float knobX = trackX + ratio * trackW;

    DrawLine(trackX, trackY, trackX + trackW, trackY, Theme::Divider, 3.0f);
    DrawLine(trackX, trackY, knobX, trackY, accentColor_, 3.0f);
    FillRound(knobX - 5.0f, trackY - 5.0f, knobX + 5.0f, trackY + 5.0f,
        5.0f, activeSlider_ == sliderId ? AccentBright() : MakeColor(0xE7E7E7));

    const bool hovered = Hover(trackX - 5.0f, trackY - 9.0f,
        trackX + trackW + 5.0f, trackY + 9.0f);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && hovered && mousePressed_)
        activeSlider_ = sliderId;

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && mouseDown_ && activeSlider_ == sliderId)
    {
        const float t = Clamp((static_cast<float>(mouse_.x) - trackX) / trackW, 0.0f, 1.0f);
        value = minimum + t * (maximum - minimum);
        if (!isFloat)
            value = std::round(value);
    }
}

void App::Dropdown(const std::wstring& label, const std::vector<std::wstring>& items, int& selected, float x, float y, float w)
{
    if (items.empty())
        return;

    selected = std::clamp(selected, 0, static_cast<int>(items.size()) - 1);

    DrawTextLine(label, x, y, x + 120.0f, y + 24.0f, Theme::Text, font_);

    const float boxX = x + 120.0f;
    const float boxW = w - 120.0f;
    const float boxTop = y;
    const float boxBottom = y + 30.0f;
    const bool hovered = Hover(boxX, boxTop, boxX + boxW, boxBottom);

    FillRound(
        boxX,
        boxTop,
        boxX + boxW,
        boxBottom,
        8.0f,
        hovered || dropdownOpen_ ? Theme::InputHover : Theme::Input);

    DrawRound(
        boxX,
        boxTop,
        boxX + boxW,
        boxBottom,
        8.0f,
        dropdownOpen_ ? WithAlpha(accentColor_, 0.75f) : Theme::CardBorder,
        1.0f);

    DrawTextLine(
        items[selected],
        boxX + 12.0f,
        boxTop,
        boxX + boxW - 28.0f,
        boxBottom,
        Theme::Text,
        font_);

    DrawIcon(
        Icon::Chevron,
        boxX + boxW - 26.0f,
        boxTop,
        boxX + boxW - 6.0f,
        boxBottom,
        dropdownOpen_ ? AccentBright() : Theme::Muted);

    if (!colorPickerOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && hovered && mousePressed_)
    {
        dropdownOpen_ = !dropdownOpen_;
        keybindCapture_ = false;
        configNameFocused_ = false;
    }

    // Store the popup for a dedicated overlay pass. The list is intentionally
    // NOT drawn here because later cards would paint over it.
    if (dropdownOpen_)
    {
        dropdownOverlay_.valid = true;
        dropdownOverlay_.buttonLeft = boxX;
        dropdownOverlay_.buttonTop = boxTop;
        dropdownOverlay_.buttonRight = boxX + boxW;
        dropdownOverlay_.buttonBottom = boxBottom;
        dropdownOverlay_.left = boxX;
        dropdownOverlay_.top = boxBottom + 4.0f;
        dropdownOverlay_.right = boxX + boxW;
        dropdownOverlay_.itemHeight = 29.0f;
        dropdownOverlay_.items = items;
        dropdownOverlay_.selected = &selected;
    }
}

void App::DrawDropdownOverlay()
{
    if (!dropdownOpen_ || !dropdownOverlay_.valid ||
        dropdownOverlay_.selected == nullptr || dropdownOverlay_.items.empty())
    {
        return;
    }

    const float left = dropdownOverlay_.left;
    const float top = dropdownOverlay_.top;
    const float right = dropdownOverlay_.right;
    const float itemHeight = dropdownOverlay_.itemHeight;
    const float bottom = top + itemHeight * static_cast<float>(dropdownOverlay_.items.size());

    // Opaque popup plus a small shadow makes it visually top-most over every
    // card and prevents text from the UI below showing through.
    FillRound(left + 4.0f, top + 5.0f, right + 4.0f, bottom + 5.0f,
        8.0f, PopupColor(MakeColor(0x000000, 0.42f)));
    FillRound(left, top, right, bottom, 8.0f, PopupColor(MakeColor(0x090B0E)));
    DrawRound(left, top, right, bottom, 8.0f,
        WithAlpha(accentColor_, 0.72f), 1.0f);

    for (std::size_t index = 0; index < dropdownOverlay_.items.size(); ++index)
    {
        const float itemTop = top + itemHeight * static_cast<float>(index);
        const float itemBottom = itemTop + itemHeight;
        const bool hovered = Hover(left, itemTop, right, itemBottom);
        const bool selected = static_cast<int>(index) == *dropdownOverlay_.selected;

        if (hovered)
        {
            FillRound(
                left + 3.0f,
                itemTop + 2.0f,
                right - 3.0f,
                itemBottom - 2.0f,
                5.0f,
                Theme::InputHover);
        }

        if (selected)
        {
            FillRound(
                left + 5.0f,
                itemTop + 7.0f,
                left + 8.0f,
                itemBottom - 7.0f,
                1.5f,
                accentColor_);
        }

        DrawTextLine(
            dropdownOverlay_.items[index],
            left + 13.0f,
            itemTop,
            right - 12.0f,
            itemBottom,
            selected ? AccentBright() : hovered ? Theme::Text : Theme::Muted,
            font_);

        // The dropdown overlay is the top-most modal control. Its own items
        // must remain clickable even when the parent popup owns the click.
        if (!colorPickerOpen_ && hovered && mousePressed_)
        {
            *dropdownOverlay_.selected = static_cast<int>(index);
            dropdownOpen_ = false;
            dropdownOverlay_ = DropdownOverlay{};
            mousePressed_ = false;
            return;
        }
    }
}

bool App::IsModifierKey(int virtualKey) const
{
    return virtualKey == VK_CONTROL ||
        virtualKey == VK_LCONTROL ||
        virtualKey == VK_RCONTROL ||
        virtualKey == VK_MENU ||
        virtualKey == VK_LMENU ||
        virtualKey == VK_RMENU ||
        virtualKey == VK_SHIFT ||
        virtualKey == VK_LSHIFT ||
        virtualKey == VK_RSHIFT;
}

int App::EncodeHotkey(int virtualKey) const
{
    int encoded = virtualKey & HotkeyKeyMask;

    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
        encoded |= HotkeyCtrlFlag;

    if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0)
        encoded |= HotkeyAltFlag;

    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
        encoded |= HotkeyShiftFlag;

    return encoded;
}

std::wstring App::BaseKeyName(int virtualKey) const
{
    if (virtualKey == 0)
        return L"None";

    switch (virtualKey)
    {
    case VK_LBUTTON: return L"Mouse 1";
    case VK_RBUTTON: return L"Mouse 2";
    case VK_MBUTTON: return L"Mouse 3";
    case VK_XBUTTON1: return L"Mouse 4";
    case VK_XBUTTON2: return L"Mouse 5";
    case VK_INSERT: return L"Insert";
    case VK_DELETE: return L"Delete";
    case VK_HOME: return L"Home";
    case VK_END: return L"End";
    case VK_PRIOR: return L"Page Up";
    case VK_NEXT: return L"Page Down";
    case VK_SPACE: return L"Space";
    case VK_TAB: return L"Tab";
    case VK_RETURN: return L"Enter";
    case VK_ESCAPE: return L"Escape";
    case VK_BACK: return L"Backspace";
    default:
        break;
    }

    UINT scanCode = MapVirtualKeyW(
        static_cast<UINT>(virtualKey),
        MAPVK_VK_TO_VSC);

    if (virtualKey == VK_LEFT ||
        virtualKey == VK_UP ||
        virtualKey == VK_RIGHT ||
        virtualKey == VK_DOWN ||
        virtualKey == VK_INSERT ||
        virtualKey == VK_DELETE ||
        virtualKey == VK_HOME ||
        virtualKey == VK_END ||
        virtualKey == VK_PRIOR ||
        virtualKey == VK_NEXT)
    {
        scanCode |= 0xE000;
    }

    wchar_t name[64]{};
    if (GetKeyNameTextW(
        static_cast<LONG>(scanCode << 16),
        name,
        static_cast<int>(sizeof(name) / sizeof(name[0]))) > 0)
    {
        return name;
    }

    return L"Key " + std::to_wstring(virtualKey);
}

std::wstring App::KeybindName(int encodedKey) const
{
    const int virtualKey = encodedKey & HotkeyKeyMask;
    if (virtualKey == 0)
        return L"None";

    std::wstring result;

    if ((encodedKey & HotkeyCtrlFlag) != 0)
        result += L"Ctrl+";

    if ((encodedKey & HotkeyAltFlag) != 0)
        result += L"Alt+";

    if ((encodedKey & HotkeyShiftFlag) != 0)
        result += L"Shift+";

    result += BaseKeyName(virtualKey);
    return result;
}

bool App::MatchesHotkey(int encodedKey, int virtualKey) const
{
    if ((encodedKey & HotkeyKeyMask) == 0 ||
        (encodedKey & HotkeyKeyMask) != (virtualKey & HotkeyKeyMask))
    {
        return false;
    }

    const bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool altDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

    const bool needsCtrl = (encodedKey & HotkeyCtrlFlag) != 0;
    const bool needsAlt = (encodedKey & HotkeyAltFlag) != 0;
    const bool needsShift = (encodedKey & HotkeyShiftFlag) != 0;

    return ctrlDown == needsCtrl &&
        altDown == needsAlt &&
        shiftDown == needsShift;
}

void App::UpdateRegisteredHotkey()
{
    UnregisterHotKey(hwnd_, 1);
    UnregisterHotKey(hwnd_, 2);
    UnregisterHotKey(hwnd_, 3);
    hotkeyRegistered_ = false;

    auto registerEncoded = [&](int id, int encodedKey) -> bool
        {
            const int virtualKey = encodedKey & HotkeyKeyMask;
            if (virtualKey == 0)
                return false;

            UINT modifiers = MOD_NOREPEAT;
            if ((encodedKey & HotkeyCtrlFlag) != 0) modifiers |= MOD_CONTROL;
            if ((encodedKey & HotkeyAltFlag) != 0) modifiers |= MOD_ALT;
            if ((encodedKey & HotkeyShiftFlag) != 0) modifiers |= MOD_SHIFT;

            return RegisterHotKey(hwnd_, id, modifiers,
                static_cast<UINT>(virtualKey)) != FALSE;
        };

    hotkeyRegistered_ = registerEncoded(1, aimbotKeybind_);
    registerEncoded(2, openCloseHotkey_);
    registerEncoded(3, exitHotkey_);
}

void App::KeybindControl(
    const std::wstring& label,
    int& encodedKey,
    float x,
    float y,
    float w)
{
    const float rowHeight = 31.0f;
    const float fieldWidth = 118.0f;
    const float fieldLeft = x + w - fieldWidth;
    const float fieldRight = x + w;

    const bool capturingThis = keybindCapture_ && keybindCaptureTarget_ == &encodedKey;
    const bool rowHover = Hover(x, y, x + w, y + rowHeight);
    const bool fieldHover = Hover(fieldLeft, y + 2.0f, fieldRight, y + rowHeight - 2.0f);

    if (rowHover)
    {
        FillRound(
            x - 5.0f,
            y + 1.0f,
            x + w + 1.0f,
            y + rowHeight - 1.0f,
            5.0f,
            WithAlpha(Theme::RowHover, 0.28f));
    }

    DrawTextLine(
        label,
        x,
        y,
        fieldLeft - 10.0f,
        y + rowHeight,
        rowHover ? Theme::Text : MakeColor(0xD5D5D5),
        font_);

    FillRound(
        fieldLeft,
        y + 4.0f,
        fieldRight,
        y + rowHeight - 4.0f,
        6.0f,
        capturingThis
        ? WithAlpha(accentColor_, 0.22f)
        : fieldHover
        ? Theme::InputHover
        : Theme::Input);

    DrawRound(
        fieldLeft,
        y + 4.0f,
        fieldRight,
        y + rowHeight - 4.0f,
        6.0f,
        capturingThis ? accentColor_ : Theme::CardBorder,
        1.0f);

    const std::wstring valueText = capturingThis
        ? L"Press key..."
        : KeybindName(encodedKey);

    DrawTextLine(
        valueText,
        fieldLeft + 7.0f,
        y + 4.0f,
        fieldRight - 7.0f,
        y + rowHeight - 4.0f,
        capturingThis ? AccentBright() : Theme::Text,
        smallFont_,
        DWRITE_TEXT_ALIGNMENT_CENTER);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && fieldHover && mousePressed_)
    {
        keybindCapture_ = true;
        keybindCaptureTarget_ = &encodedKey;
        dropdownOpen_ = false;
        SetForegroundWindow(hwnd_);
        SetFocus(hwnd_);
    }
}


bool App::Button(
    const std::wstring& label,
    const wchar_t* icon,
    float x,
    float y,
    float w,
    float h,
    bool accent,
    bool danger)
{
    const bool hovered = Hover(x, y, x + w, y + h);

    D2D1_COLOR_F fill = hovered ? Theme::InputHover : Theme::Input;
    D2D1_COLOR_F border = Theme::CardBorder;
    D2D1_COLOR_F textColor = hovered ? Theme::Text : MakeColor(0xD5D5D5);

    if (accent)
    {
        fill = hovered ? AccentBright() : accentColor_;
        border = AccentBright();
        textColor = MakeColor(0xFFFFFF);
    }
    else if (danger)
    {
        fill = hovered ? MakeColor(0x54151C) : MakeColor(0x261014);
        border = MakeColor(0x7D1B25);
        textColor = hovered ? MakeColor(0xFFFFFF) : MakeColor(0xDF8A92);
    }

    FillRound(x, y, x + w, y + h, 7.0f, fill);
    DrawRound(x, y, x + w, y + h, 7.0f, border, 1.0f);

    if (icon)
        DrawIcon(icon, x + 8.0f, y, x + 32.0f, y + h, textColor);

    DrawTextLine(
        label,
        x + (icon ? 35.0f : 8.0f),
        y,
        x + w - 8.0f,
        y + h,
        textColor,
        smallFont_,
        icon ? DWRITE_TEXT_ALIGNMENT_LEADING : DWRITE_TEXT_ALIGNMENT_CENTER);

    return !colorPickerOpen_ && !dropdownOpen_ && hovered && mousePressed_;
}

void App::TextInput(
    const std::wstring& label,
    std::wstring& value,
    bool& focused,
    float x,
    float y,
    float w)
{
    DrawTextLine(label, x, y, x + 110.0f, y + 31.0f, Theme::Text, font_);

    const float inputX = x + 112.0f;
    const float inputW = w - 112.0f;
    const bool hovered = Hover(inputX, y, inputX + inputW, y + 31.0f);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && mousePressed_)
        focused = hovered;

    FillRound(
        inputX,
        y,
        inputX + inputW,
        y + 31.0f,
        7.0f,
        focused || hovered ? Theme::InputHover : Theme::Input);

    DrawRound(
        inputX,
        y,
        inputX + inputW,
        y + 31.0f,
        7.0f,
        focused ? accentColor_ : Theme::CardBorder,
        1.0f);

    DrawTextLine(
        value.empty() ? L"Enter config name" : value,
        inputX + 10.0f,
        y,
        inputX + inputW - 10.0f,
        y + 31.0f,
        value.empty() ? Theme::Muted2 : Theme::Text,
        smallFont_);

    if (focused && ((GetTickCount64() / 500ULL) % 2ULL) == 0ULL)
    {
        const float caretX = MinValue(
            inputX + 11.0f + static_cast<float>(value.size()) * 6.2f,
            inputX + inputW - 12.0f);
        DrawLine(caretX, y + 8.0f, caretX, y + 23.0f, AccentBright(), 1.0f);
    }
}

std::filesystem::path App::ConfigDirectory() const
{
    wchar_t appData[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);

    std::filesystem::path directory = length > 0
        ? std::filesystem::path(appData) / L"RenderOneUI" / L"Configs"
        : std::filesystem::current_path() / L"Configs";

    std::error_code error;
    std::filesystem::create_directories(directory, error);
    return directory;
}

std::wstring App::CleanConfigName(const std::wstring& name) const
{
    std::wstring clean;
    clean.reserve(name.size());

    for (wchar_t character : name)
    {
        if (std::iswalnum(character) ||
            character == L' ' ||
            character == L'-' ||
            character == L'_')
        {
            clean.push_back(character);
        }
    }

    while (!clean.empty() && clean.front() == L' ')
        clean.erase(clean.begin());
    while (!clean.empty() && clean.back() == L' ')
        clean.pop_back();

    if (clean.size() > 36)
        clean.resize(36);

    return clean;
}

std::filesystem::path App::ConfigPath(const std::wstring& name) const
{
    return ConfigDirectory() / (CleanConfigName(name) + L".cfg");
}

bool App::SaveConfig(const std::wstring& name)
{
    const std::wstring cleanName = CleanConfigName(name);
    if (cleanName.empty())
        return false;

    std::wofstream file(ConfigPath(cleanName), std::ios::trunc);
    if (!file)
        return false;

    auto writeBool = [&](const wchar_t* key, bool value)
        {
            file << key << L"=" << (value ? 1 : 0) << L"\n";
        };
    auto writeFloat = [&](const wchar_t* key, float value)
        {
            file << key << L"=" << value << L"\n";
        };
    auto writeInt = [&](const wchar_t* key, int value)
        {
            file << key << L"=" << value << L"\n";
        };
    auto writeColor = [&](const wchar_t* prefix, const D2D1_COLOR_F& color)
        {
            file << prefix << L"R=" << color.r << L"\n";
            file << prefix << L"G=" << color.g << L"\n";
            file << prefix << L"B=" << color.b << L"\n";
        };

    file << L"[RenderOneUI]\n";
    writeBool(L"aimEnabled", aimEnabled_);
    writeBool(L"silentAim", silentAim_);
    writeBool(L"teamCheck", teamCheck_);
    writeBool(L"visibleOnly", visibleOnly_);
    writeBool(L"prediction", prediction_);
    writeFloat(L"aimFov", aimFov_);
    writeFloat(L"aimSmooth", aimSmooth_);
    writeFloat(L"aimDistance", aimDistance_);
    writeInt(L"targetBone", targetBone_);
    writeInt(L"aimbotKeybind", aimbotKeybind_);
    writeInt(L"openCloseHotkey", openCloseHotkey_);
    writeInt(L"exitHotkey", exitHotkey_);
    writeInt(L"aimPriority", aimPriority_);
    writeBool(L"aimAutoSwitch", aimAutoSwitch_);
    writeBool(L"aimIgnoreDowned", aimIgnoreDowned_);
    writeFloat(L"aimStartDelay", aimStartDelay_);
    writeInt(L"silentMode", silentMode_);
    writeBool(L"silentVisibleFallback", silentVisibleFallback_);
    writeBool(L"silentThroughSmoke", silentThroughSmoke_);
    writeFloat(L"silentHitChance", silentHitChance_);
    writeInt(L"predictionMode", predictionMode_);
    writeBool(L"predictionGravity", predictionGravity_);
    writeBool(L"predictionAirborne", predictionAirborne_);
    writeFloat(L"predictionStrength", predictionStrength_);

    writeBool(L"espBox", espBox_);
    writeBool(L"espCornerBox", espCornerBox_);
    writeBool(L"espHealthBar", espHealthBar_);
    writeBool(L"espName", espName_);
    writeBool(L"espDistance", espDistance_);
    writeBool(L"espSnapline", espSnapline_);
    writeBool(L"espSkeleton", espSkeleton_);
    writeBool(L"espWeapon", espWeapon_);
    writeBool(L"espFilled", espFilled_);
    writeBool(L"espDamage", espDamage_);
    writeBool(L"visualGlow", visualGlow_);
    writeFloat(L"visualGlowThickness", visualGlowThickness_);

    writeBool(L"particlesEnabled", particlesEnabled_);
    writeFloat(L"particleSpeed", particleSpeed_);
    writeFloat(L"particleAmount", particleAmount_);
    writeFloat(L"previewScale", previewScale_);
    writeFloat(L"windowOpacity", windowOpacity_);
    writeFloat(L"popupOpacity", popupOpacity_);
    writeInt(L"healthStyle", healthStyle_);
    writeFloat(L"healthBarWidth", healthBarWidth_);

    writeColor(L"background", backgroundColor_);
    writeColor(L"accent", accentColor_);
    writeColor(L"box", boxColor_);
    writeColor(L"filled", filledColor_);
    writeColor(L"health", healthColor_);
    writeColor(L"healthBack", healthBackColor_);
    writeColor(L"damage", damageColor_);
    writeColor(L"visualGlow", visualGlowColor_);
    writeColor(L"line", lineColor_);
    writeColor(L"skeleton", skeletonColor_);

    file.flush();
    return file.good();
}

bool App::LoadConfig(const std::wstring& name)
{
    std::wifstream file(ConfigPath(name));
    if (!file)
        return false;

    std::wstring line;
    while (std::getline(file, line))
    {
        if (line.empty() || line.front() == L'[')
            continue;

        const std::size_t separator = line.find(L'=');
        if (separator == std::wstring::npos)
            continue;

        const std::wstring key = line.substr(0, separator);
        const std::wstring value = line.substr(separator + 1);

        try
        {
            const bool booleanValue = std::stoi(value) != 0;
            const float floatValue = std::stof(value);

            if (key == L"aimEnabled") aimEnabled_ = booleanValue;
            else if (key == L"silentAim") silentAim_ = booleanValue;
            else if (key == L"teamCheck") teamCheck_ = booleanValue;
            else if (key == L"visibleOnly") visibleOnly_ = booleanValue;
            else if (key == L"prediction") prediction_ = booleanValue;
            else if (key == L"aimFov") aimFov_ = floatValue;
            else if (key == L"aimSmooth") aimSmooth_ = floatValue;
            else if (key == L"aimDistance") aimDistance_ = floatValue;
            else if (key == L"targetBone") targetBone_ = std::stoi(value);
            else if (key == L"aimbotKeybind") aimbotKeybind_ = std::stoi(value);
            else if (key == L"openCloseHotkey") openCloseHotkey_ = std::stoi(value);
            else if (key == L"exitHotkey") exitHotkey_ = std::stoi(value);
            else if (key == L"aimPriority") aimPriority_ = std::stoi(value);
            else if (key == L"aimAutoSwitch") aimAutoSwitch_ = booleanValue;
            else if (key == L"aimIgnoreDowned") aimIgnoreDowned_ = booleanValue;
            else if (key == L"aimStartDelay") aimStartDelay_ = floatValue;
            else if (key == L"silentMode") silentMode_ = std::stoi(value);
            else if (key == L"silentVisibleFallback") silentVisibleFallback_ = booleanValue;
            else if (key == L"silentThroughSmoke") silentThroughSmoke_ = booleanValue;
            else if (key == L"silentHitChance") silentHitChance_ = floatValue;
            else if (key == L"predictionMode") predictionMode_ = std::stoi(value);
            else if (key == L"predictionGravity") predictionGravity_ = booleanValue;
            else if (key == L"predictionAirborne") predictionAirborne_ = booleanValue;
            else if (key == L"predictionStrength") predictionStrength_ = floatValue;

            else if (key == L"espBox") espBox_ = booleanValue;
            else if (key == L"espCornerBox") espCornerBox_ = booleanValue;
            else if (key == L"espHealthBar") espHealthBar_ = booleanValue;
            else if (key == L"espName") espName_ = booleanValue;
            else if (key == L"espDistance") espDistance_ = booleanValue;
            else if (key == L"espSnapline") espSnapline_ = booleanValue;
            else if (key == L"espSkeleton") espSkeleton_ = booleanValue;
            else if (key == L"espWeapon") espWeapon_ = booleanValue;
            else if (key == L"espFilled") espFilled_ = booleanValue;
            else if (key == L"espDamage") espDamage_ = booleanValue;
            else if (key == L"visualGlow") visualGlow_ = booleanValue;
            else if (key == L"visualGlowThickness") visualGlowThickness_ = floatValue;

            else if (key == L"particlesEnabled") particlesEnabled_ = booleanValue;
            else if (key == L"particleSpeed") particleSpeed_ = floatValue;
            else if (key == L"particleAmount") particleAmount_ = floatValue;
            else if (key == L"previewScale") previewScale_ = floatValue;
            else if (key == L"windowOpacity") windowOpacity_ = Clamp(floatValue, 80.0f, 255.0f);
            else if (key == L"popupOpacity") popupOpacity_ = Clamp(floatValue, 64.0f, 255.0f);
            else if (key == L"healthStyle") healthStyle_ = std::stoi(value);
            else if (key == L"healthBarWidth") healthBarWidth_ = floatValue;

            else if (key == L"backgroundR") backgroundColor_.r = floatValue;
            else if (key == L"backgroundG") backgroundColor_.g = floatValue;
            else if (key == L"backgroundB") backgroundColor_.b = floatValue;
            else if (key == L"accentR") accentColor_.r = floatValue;
            else if (key == L"accentG") accentColor_.g = floatValue;
            else if (key == L"accentB") accentColor_.b = floatValue;
            else if (key == L"boxR") boxColor_.r = floatValue;
            else if (key == L"boxG") boxColor_.g = floatValue;
            else if (key == L"boxB") boxColor_.b = floatValue;
            else if (key == L"healthR") healthColor_.r = floatValue;
            else if (key == L"healthG") healthColor_.g = floatValue;
            else if (key == L"healthB") healthColor_.b = floatValue;
            else if (key == L"filledR") filledColor_.r = floatValue;
            else if (key == L"filledG") filledColor_.g = floatValue;
            else if (key == L"filledB") filledColor_.b = floatValue;
            else if (key == L"healthBackR") healthBackColor_.r = floatValue;
            else if (key == L"healthBackG") healthBackColor_.g = floatValue;
            else if (key == L"healthBackB") healthBackColor_.b = floatValue;
            else if (key == L"damageR") damageColor_.r = floatValue;
            else if (key == L"damageG") damageColor_.g = floatValue;
            else if (key == L"damageB") damageColor_.b = floatValue;
            else if (key == L"visualGlowR") visualGlowColor_.r = floatValue;
            else if (key == L"visualGlowG") visualGlowColor_.g = floatValue;
            else if (key == L"visualGlowB") visualGlowColor_.b = floatValue;
            else if (key == L"lineR") lineColor_.r = floatValue;
            else if (key == L"lineG") lineColor_.g = floatValue;
            else if (key == L"lineB") lineColor_.b = floatValue;
            else if (key == L"skeletonR") skeletonColor_.r = floatValue;
            else if (key == L"skeletonG") skeletonColor_.g = floatValue;
            else if (key == L"skeletonB") skeletonColor_.b = floatValue;
        }
        catch (...)
        {
        }
    }

    aimFov_ = Clamp(aimFov_, 1.0f, 180.0f);
    aimSmooth_ = Clamp(aimSmooth_, 0.0f, 30.0f);
    aimDistance_ = Clamp(aimDistance_, 10.0f, 1000.0f);
    targetBone_ = std::clamp(targetBone_, 0, 4);
    particleSpeed_ = Clamp(particleSpeed_, 4.0f, 60.0f);
    particleAmount_ = Clamp(particleAmount_, 8.0f, 70.0f);
    healthStyle_ = std::clamp(healthStyle_, 0, 1);
    healthBarWidth_ = Clamp(healthBarWidth_, 6.0f, 20.0f);
    visualGlowThickness_ = Clamp(visualGlowThickness_, 0.5f, 6.0f);

    auto clampColor = [](D2D1_COLOR_F& color)
        {
            color.r = Clamp(color.r, 0.0f, 1.0f);
            color.g = Clamp(color.g, 0.0f, 1.0f);
            color.b = Clamp(color.b, 0.0f, 1.0f);
            color.a = 1.0f;
        };

    clampColor(backgroundColor_);
    clampColor(accentColor_);
    clampColor(boxColor_);
    clampColor(healthColor_);
    clampColor(lineColor_);
    clampColor(skeletonColor_);

    UpdateRegisteredHotkey();
    UpdateRegisteredHotkey();
    return true;
}

bool App::DeleteConfig(const std::wstring& name)
{
    std::error_code error;
    return std::filesystem::remove(ConfigPath(name), error);
}

void App::RefreshConfigs()
{
    const std::wstring previous =
        selectedConfig_ >= 0 && selectedConfig_ < static_cast<int>(configs_.size())
        ? configs_[selectedConfig_]
        : L"";

    configs_.clear();
    std::error_code error;

    for (const auto& entry : std::filesystem::directory_iterator(ConfigDirectory(), error))
    {
        if (error)
            break;

        if (entry.is_regular_file() && entry.path().extension() == L".cfg")
            configs_.push_back(entry.path().stem().wstring());
    }

    std::sort(configs_.begin(), configs_.end());
    selectedConfig_ = -1;

    if (!previous.empty())
    {
        const auto iterator = std::find(configs_.begin(), configs_.end(), previous);
        if (iterator != configs_.end())
            selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), iterator));
    }

    if (selectedConfig_ < 0 && !configs_.empty())
        selectedConfig_ = 0;

    const float maxScroll = MaxValue(0.0f, static_cast<float>(configs_.size()) - 6.0f);
    configScroll_ = Clamp(configScroll_, 0.0f, maxScroll);
}

void App::ShowConfigStatus(const std::wstring& text, bool ok)
{
    configStatus_ = text;
    configStatusOk_ = ok;
    configStatusTime_ = 3.0f;
}

void App::ColorRow(const std::wstring& label, D2D1_COLOR_F& color, float x, float y, float w, bool second)
{
    const float rowH = 29.0f;
    const float iconLeft = x + w - 31.0f;
    const float iconRight = x + w - 2.0f;
    const bool rowHover = Hover(x, y, x + w, y + rowH);
    const bool iconHover = Hover(iconLeft, y, iconRight, y + rowH);

    if (rowHover)
        FillRound(x - 5.0f, y + 1.0f, x + w + 1.0f, y + rowH - 1.0f, 5.0f,
            WithAlpha(Theme::RowHover, 0.32f));

    DrawTextLine(label, x, y, x + w - 36.0f, y + rowH,
        rowHover ? Theme::Text : MakeColor(0xD5D5D5), font_);

    if (iconHover)
        FillRound(iconLeft + 2.0f, y + 3.0f, iconRight - 1.0f, y + rowH - 3.0f,
            6.0f, WithAlpha(color, 0.12f));

    DrawIcon(Icon::Drop, iconLeft, y + (second ? -1.0f : 0.0f), iconRight, y + rowH,
        color);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && iconHover && mousePressed_)
        OpenColorPicker(color);
}

void App::OpenColorPicker(D2D1_COLOR_F& color)
{
    pickerTarget_ = &color;
    pickerOriginal_ = color;
    pickerWorking_ = color;
    ColorToHsv(color, pickerHue_, pickerSaturation_, pickerValue_);
    colorPickerOpen_ = true;
    activeColorArea_ = -1;
    activeSlider_ = -1;
    dropdownOpen_ = false;
}

void App::UpdatePickerColorFromHsv()
{
    pickerWorking_ = HsvToColor(pickerHue_, pickerSaturation_, pickerValue_);
}

void App::DrawColorPicker()
{
    if (!colorPickerOpen_ || !pickerTarget_)
        return;

    FillRect(0.0f, 0.0f, static_cast<float>(UI::Width),
        static_cast<float>(UI::Height), MakeColor(0x000000, 0.76f));

    const float boxWidth = 364.0f;
    const float boxHeight = 300.0f;
    const float x = (static_cast<float>(UI::Width) - boxWidth) * 0.5f;
    const float y = (static_cast<float>(UI::Height) - boxHeight) * 0.5f;

    FillRound(x, y, x + boxWidth, y + boxHeight, 12.0f, PopupColor(MakeColor(0x090A0D)));
    DrawRound(x, y, x + boxWidth, y + boxHeight, 12.0f, Theme::CardBorder, 1.0f);
    DrawLine(x + 15.0f, y + 42.0f, x + boxWidth - 15.0f, y + 42.0f, accentColor_, 1.2f);

    DrawTextLine(L"Custom Color Picker", x + 16.0f, y + 5.0f,
        x + boxWidth - 16.0f, y + 38.0f, Theme::Text, titleFont_);

    const float paletteX = x + 18.0f;
    const float paletteY = y + 57.0f;
    const float paletteWidth = 274.0f;
    const float paletteHeight = 174.0f;
    const float hueX = paletteX + paletteWidth + 11.0f;
    const float hueWidth = 20.0f;

    constexpr int columns = 64;
    constexpr int rows = 42;
    const float cellW = paletteWidth / static_cast<float>(columns);
    const float cellH = paletteHeight / static_cast<float>(rows);

    for (int row = 0; row < rows; ++row)
    {
        const float value = 1.0f - static_cast<float>(row) / static_cast<float>(rows - 1);
        for (int column = 0; column < columns; ++column)
        {
            const float saturation = static_cast<float>(column) / static_cast<float>(columns - 1);
            const float left = paletteX + cellW * static_cast<float>(column);
            const float top = paletteY + cellH * static_cast<float>(row);
            FillRect(left, top, left + cellW + 0.8f, top + cellH + 0.8f,
                HsvToColor(pickerHue_, saturation, value));
        }
    }

    constexpr int hueSteps = 96;
    const float hueStep = paletteHeight / static_cast<float>(hueSteps);
    for (int i = 0; i < hueSteps; ++i)
    {
        const float hue = static_cast<float>(i) / static_cast<float>(hueSteps - 1);
        const float top = paletteY + hueStep * static_cast<float>(i);
        FillRect(hueX, top, hueX + hueWidth, top + hueStep + 0.8f,
            HsvToColor(hue, 1.0f, 1.0f));
    }

    DrawRect(paletteX, paletteY, paletteX + paletteWidth, paletteY + paletteHeight,
        MakeColor(0x202329), 1.0f);
    DrawRect(hueX, paletteY, hueX + hueWidth, paletteY + paletteHeight,
        MakeColor(0x202329), 1.0f);

    const bool paletteHover = Hover(paletteX, paletteY,
        paletteX + paletteWidth, paletteY + paletteHeight);
    const bool hueHover = Hover(hueX, paletteY,
        hueX + hueWidth, paletteY + paletteHeight);

    if (mousePressed_ && paletteHover)
        activeColorArea_ = 0;
    else if (mousePressed_ && hueHover)
        activeColorArea_ = 1;

    if (mouseDown_ && activeColorArea_ == 0 && paletteHover)
    {
        pickerSaturation_ = Clamp(
            (static_cast<float>(mouse_.x) - paletteX) / paletteWidth, 0.0f, 1.0f);
        pickerValue_ = 1.0f - Clamp(
            (static_cast<float>(mouse_.y) - paletteY) / paletteHeight, 0.0f, 1.0f);
        UpdatePickerColorFromHsv();
    }
    else if (mouseDown_ && activeColorArea_ == 1 && hueHover)
    {
        pickerHue_ = Clamp(
            (static_cast<float>(mouse_.y) - paletteY) / paletteHeight, 0.0f, 1.0f);
        UpdatePickerColorFromHsv();
    }

    const float pointX = paletteX + pickerSaturation_ * paletteWidth;
    const float pointY = paletteY + (1.0f - pickerValue_) * paletteHeight;

    SetBrush(MakeColor(0x000000));
    target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(pointX, pointY), 5.5f, 5.5f), brush_, 3.0f);
    SetBrush(MakeColor(0xFFFFFF));
    target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(pointX, pointY), 5.5f, 5.5f), brush_, 1.4f);

    const float hueMarkerY = paletteY + pickerHue_ * paletteHeight;
    FillRect(hueX - 3.0f, hueMarkerY - 2.0f,
        hueX + hueWidth + 3.0f, hueMarkerY + 2.0f, MakeColor(0xFFFFFF));
    DrawRect(hueX - 4.0f, hueMarkerY - 3.0f,
        hueX + hueWidth + 4.0f, hueMarkerY + 3.0f, MakeColor(0x000000), 1.0f);

    FillRound(x + 18.0f, y + 244.0f, x + 49.0f, y + 266.0f, 4.0f, pickerWorking_);
    DrawRound(x + 18.0f, y + 244.0f, x + 49.0f, y + 266.0f, 4.0f,
        WithAlpha(MakeColor(0xFFFFFF), 0.18f), 1.0f);

    wchar_t hexText[16]{};
    const int red = std::clamp(static_cast<int>(std::round(pickerWorking_.r * 255.0f)), 0, 255);
    const int green = std::clamp(static_cast<int>(std::round(pickerWorking_.g * 255.0f)), 0, 255);
    const int blue = std::clamp(static_cast<int>(std::round(pickerWorking_.b * 255.0f)), 0, 255);
    swprintf_s(hexText, L"#%02X%02X%02X", red, green, blue);

    DrawTextLine(hexText, x + 58.0f, y + 239.0f,
        x + 165.0f, y + 270.0f, Theme::Text, smallFont_);

    const float buttonY = y + boxHeight - 37.0f;
    const float cancelX = x + boxWidth - 158.0f;
    const float applyX = x + boxWidth - 81.0f;

    const bool cancelHover = Hover(cancelX, buttonY, cancelX + 68.0f, buttonY + 25.0f);
    const bool applyHover = Hover(applyX, buttonY, applyX + 63.0f, buttonY + 25.0f);

    FillRound(cancelX, buttonY, cancelX + 68.0f, buttonY + 25.0f, 6.0f,
        cancelHover ? Theme::InputHover : Theme::Input);
    DrawRound(cancelX, buttonY, cancelX + 68.0f, buttonY + 25.0f, 6.0f,
        Theme::CardBorder, 1.0f);
    DrawTextLine(L"Cancel", cancelX, buttonY, cancelX + 68.0f, buttonY + 25.0f,
        Theme::Text, smallFont_, DWRITE_TEXT_ALIGNMENT_CENTER);

    FillRound(applyX, buttonY, applyX + 63.0f, buttonY + 25.0f, 6.0f,
        applyHover ? AccentBright() : accentColor_);
    DrawTextLine(L"Apply", applyX, buttonY, applyX + 63.0f, buttonY + 25.0f,
        MakeColor(0xFFFFFF), smallFont_, DWRITE_TEXT_ALIGNMENT_CENTER);

    if (mousePressed_ && cancelHover)
    {
        *pickerTarget_ = pickerOriginal_;
        colorPickerOpen_ = false;
        pickerTarget_ = nullptr;
        activeColorArea_ = -1;
    }
    else if (mousePressed_ && applyHover)
    {
        *pickerTarget_ = pickerWorking_;
        colorPickerOpen_ = false;
        pickerTarget_ = nullptr;
        activeColorArea_ = -1;
    }
}

void App::DrawAimbotFeatureRow(const std::wstring& label, bool& value, int popupId, float x, float y, float w)
{
    const float gearSize = 22.0f;
    const float gearX = x;
    const float gearY = y + 3.0f;
    const bool gearHover = Hover(gearX, gearY, gearX + gearSize, gearY + gearSize);
    const bool active = activeAimbotPopup_ == popupId;

    if (gearHover || active)
        FillRound(gearX, gearY, gearX + gearSize, gearY + gearSize, 5.0f,
            active ? AccentSoft(0.38f) : WithAlpha(Theme::RowHover, 0.55f));

    DrawIcon(Icon::Gear,
        gearX + 3.0f, gearY + 3.0f,
        gearX + gearSize - 3.0f, gearY + gearSize - 3.0f,
        active ? AccentBright() : gearHover ? Theme::Text : Theme::Muted);

    if (!colorPickerOpen_ && !dropdownOpen_ && !popupOwnsClick_ && gearHover && mousePressed_)
    {
        activeAimbotPopup_ = active ? 0 : popupId;
        activeVisualPopup_ = 0;
        configOpen_ = false;
        dropdownOpen_ = false;
        dropdownOverlay_ = DropdownOverlay{};
        mousePressed_ = false;
    }

    Toggle(label, value, x + 28.0f, y, w - 28.0f);
}

void App::DrawAimbotCard(float x, float y, float w, float h)
{
    Card(x, y, w, h, L"Aimbot");

    // Only selected Aimbot rows have their own settings gear.
    DrawAimbotFeatureRow(L"Enabled", aimEnabled_, 1, x + 12.0f, y + 47.0f, w - 24.0f);
    DrawAimbotFeatureRow(L"Silent aim", silentAim_, 2, x + 12.0f, y + 78.0f, w - 24.0f);
    Toggle(L"Team check", teamCheck_, x + 12.0f, y + 109.0f, w - 24.0f);
    Toggle(L"Visible only", visibleOnly_, x + 12.0f, y + 140.0f, w - 24.0f);
    DrawAimbotFeatureRow(L"Prediction", prediction_, 3, x + 12.0f, y + 171.0f, w - 24.0f);

    Divider(x + 12.0f, y + 207.0f, w - 24.0f);

    Dropdown(L"Target bone",
        { L"Head", L"Neck", L"Chest", L"Stomach", L"Closest" },
        targetBone_, x + 12.0f, y + 222.0f, w - 24.0f);

    Slider(L"Field of view", aimFov_, 1.0f, 180.0f, false,
        x + 12.0f, y + 274.0f, w - 24.0f, 1);
    Slider(L"Smoothing", aimSmooth_, 0.0f, 30.0f, true,
        x + 12.0f, y + 312.0f, w - 24.0f, 2);
    Slider(L"Max distance", aimDistance_, 10.0f, 1000.0f, false,
        x + 12.0f, y + 350.0f, w - 24.0f, 3);

    KeybindControl(L"Activation key", aimbotKeybind_,
        x + 12.0f, y + 393.0f, w - 24.0f);
}

void App::DrawAimbotPopup(float cardX, float cardY, float cardW)
{
    aimbotPopupBoundsValid_ = false;
    if (aimbotPopupAnim_ <= 0.01f || activeAimbotPopup_ == 0)
        return;

    const float eased = 1.0f - std::pow(1.0f - aimbotPopupAnim_, 3.0f);
    const float width = 280.0f;
    const float targetHeight = 198.0f;
    const float height = targetHeight * eased;
    const float left = cardX + 36.0f;

    float rowY = 47.0f;
    const wchar_t* title = L"Enabled settings";
    if (activeAimbotPopup_ == 2)
    {
        rowY = 78.0f;
        title = L"Silent aim settings";
    }
    else if (activeAimbotPopup_ == 3)
    {
        rowY = 171.0f;
        title = L"Prediction settings";
    }

    float top = cardY + rowY + 30.0f;
    // Keep the lowest popup fully inside the window.
    if (top + targetHeight > static_cast<float>(UI::Height) - 10.0f)
        top = static_cast<float>(UI::Height) - targetHeight - 10.0f;

    const float right = left + width;
    const float bottom = top + height;

    aimbotPopupBounds_ = D2D1::RectF(left, top, right, bottom);
    aimbotPopupBoundsValid_ = true;

    FillRound(left + 4.0f, top + 5.0f, right + 4.0f, bottom + 5.0f,
        1.0f, PopupColor(MakeColor(0x000000, 0.36f)));
    FillRound(left, top, right, bottom, 1.0f, PopupColor(MakeColor(0x090B0E)));
    DrawRound(left, top, right, bottom, 1.0f,
        WithAlpha(accentColor_, 0.72f), 1.0f);

    if (height > 28.0f)
        DrawTextLine(title, left + 12.0f, top + 5.0f,
            right - 12.0f, top + 30.0f, Theme::Text, titleFont_);

    if (height < 188.0f)
        return;

    // Background controls were already drawn with popupOwnsClick_ enabled.
    // Temporarily allow only the controls inside this top-most popup to react.
    const bool oldDrawingPopup = drawingVisualPopup_;
    drawingVisualPopup_ = true;

    const float controlX = left + 12.0f;
    const float controlW = width - 24.0f;

    if (activeAimbotPopup_ == 1)
    {
        Dropdown(L"Target priority",
            { L"Closest", L"Lowest health", L"Field of view" },
            aimPriority_, controlX, top + 38.0f, controlW);
        Toggle(L"Automatic target switch", aimAutoSwitch_,
            controlX, top + 76.0f, controlW);
        Toggle(L"Ignore downed targets", aimIgnoreDowned_,
            controlX, top + 106.0f, controlW);
        Slider(L"Start delay", aimStartDelay_, 0.0f, 150.0f, false,
            controlX, top + 143.0f, controlW, 30);
    }
    else if (activeAimbotPopup_ == 2)
    {
        Dropdown(L"Mode",
            { L"Subtle", L"Balanced", L"Aggressive" },
            silentMode_, controlX, top + 38.0f, controlW);
        Toggle(L"Visible fallback", silentVisibleFallback_,
            controlX, top + 76.0f, controlW);
        Toggle(L"Allow through smoke", silentThroughSmoke_,
            controlX, top + 106.0f, controlW);
        Slider(L"Hit chance", silentHitChance_, 1.0f, 100.0f, false,
            controlX, top + 143.0f, controlW, 31);
    }
    else
    {
        Dropdown(L"Prediction method",
            { L"Linear", L"Adaptive", L"Advanced" },
            predictionMode_, controlX, top + 38.0f, controlW);
        Toggle(L"Gravity compensation", predictionGravity_,
            controlX, top + 76.0f, controlW);
        Toggle(L"Airborne prediction", predictionAirborne_,
            controlX, top + 106.0f, controlW);
        Slider(L"Strength", predictionStrength_, 0.1f, 2.0f, true,
            controlX, top + 143.0f, controlW, 32);
    }

    drawingVisualPopup_ = oldDrawingPopup;
}

void App::DrawVisualFeatureRow(const std::wstring& label, bool& value, int popupId, float x, float y, float w)
{
    const float rowH = 28.0f;
    const float gearSize = 22.0f;
    const float gearX = x;
    const float gearY = y + 3.0f;
    const bool gearHover = Hover(gearX, gearY, gearX + gearSize, gearY + gearSize);
    const bool active = activeVisualPopup_ == popupId;

    if (gearHover || active)
        FillRound(gearX, gearY, gearX + gearSize, gearY + gearSize, 5.0f,
            active ? AccentSoft(0.38f) : WithAlpha(Theme::RowHover, 0.55f));

    DrawIcon(Icon::Gear,
        gearX + 3.0f, gearY + 3.0f,
        gearX + gearSize - 3.0f, gearY + gearSize - 3.0f,
        active ? AccentBright() : gearHover ? Theme::Text : Theme::Muted);

    if (!colorPickerOpen_ && !dropdownOpen_ && (!popupOwnsClick_ || drawingVisualPopup_) && gearHover && mousePressed_)
    {
        activeVisualPopup_ = active ? 0 : popupId;
        mousePressed_ = false;
    }

    Toggle(label, value, x + 29.0f, y, w - 29.0f);
}

void App::DrawVisualFeaturePopup(float cardX, float cardY, float cardW)
{
    visualPopupBoundsValid_ = false;
    if (visualPopupAnim_ <= 0.01f || activeVisualPopup_ == 0)
        return;

    const float eased = 1.0f - std::pow(1.0f - visualPopupAnim_, 3.0f);
    const float popupW = 286.0f;
    float targetH = 88.0f;
    if (activeVisualPopup_ == 1) targetH = 158.0f;
    else if (activeVisualPopup_ == 2) targetH = 188.0f;
    else if (activeVisualPopup_ == 7) targetH = 92.0f;
    else if (activeVisualPopup_ == 8) targetH = 126.0f;

    const float rowYs[] = { 0.0f, 52.0f, 52.0f, 83.0f, 114.0f, 145.0f, 52.0f, 83.0f, 145.0f };
    const bool rightColumn = activeVisualPopup_ == 6 || activeVisualPopup_ == 7 || activeVisualPopup_ == 8;
    float anchorX = rightColumn ? cardX + cardW * 0.5f + 8.0f : cardX + 12.0f;
    float anchorY = cardY + rowYs[activeVisualPopup_];

    float left = rightColumn ? anchorX - popupW + 142.0f : anchorX + 4.0f;
    left = Clamp(left, cardX + 8.0f, cardX + cardW - popupW - 8.0f);
    const float top = anchorY + 31.0f + (1.0f - eased) * 8.0f;
    const float right = left + popupW;
    const float bottom = top + targetH * eased;
    visualPopupBounds_ = D2D1::RectF(left, top, right, bottom);
    visualPopupBoundsValid_ = true;

    FillRound(left + 4.0f, top + 6.0f, right + 4.0f, bottom + 6.0f,
        1.0f, PopupColor(MakeColor(0x000000, 0.34f * eased)));
    FillRound(left, top, right, bottom, 1.0f, PopupColor(MakeColor(0x07090C, 0.985f)));
    DrawRound(left, top, right, bottom, 1.0f, WithAlpha(accentColor_, 0.58f), 1.0f);

    if (eased < 0.94f)
        return;

    const float px = left + 14.0f;
    const float pw = popupW - 28.0f;

    drawingVisualPopup_ = true;
    switch (activeVisualPopup_)
    {
    case 1:
        DrawTextLine(L"Box settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        Dropdown(L"Style", { L"Off", L"Box", L"Cornered", L"Box + filled", L"Cornered + filled" },
            boxStyle_, px, top + 35.0f, pw);
        ColorRow(L"Outline", boxColor_, px, top + 74.0f, pw);
        ColorRow(L"Fill", filledColor_, px, top + 106.0f, pw);
        break;
    case 2:
        DrawTextLine(L"Health bar settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        Dropdown(L"Style", { L"Normal", L"Segmented" }, healthStyle_, px, top + 35.0f, pw);
        ColorRow(L"Foreground", healthColor_, px, top + 74.0f, pw);
        ColorRow(L"Background", healthBackColor_, px, top + 106.0f, pw);
        Slider(L"Bar width", healthBarWidth_, 6.0f, 20.0f, true,
            px, top + 143.0f, pw, 41);
        break;
    case 3:
        DrawTextLine(L"Name settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        DrawTextLine(L"Preview label: RenderOneUI", px, top + 35.0f, right - 14.0f, top + 65.0f, Theme::Muted, smallFont_);
        break;
    case 4:
        DrawTextLine(L"Distance settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        DrawTextLine(L"Shows the distance below the player.", px, top + 35.0f, right - 14.0f, top + 65.0f, Theme::Muted, smallFont_);
        break;
    case 5:
        DrawTextLine(L"Weapon settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        DrawTextLine(L"Shows the equipped weapon name.", px, top + 35.0f, right - 14.0f, top + 65.0f, Theme::Muted, smallFont_);
        break;
    case 6:
        DrawTextLine(L"Snap line settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        ColorRow(L"Line colour", lineColor_, px, top + 38.0f, pw);
        break;
    case 7:
        DrawTextLine(L"Damage settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        ColorRow(L"Text colour", damageColor_, px, top + 38.0f, pw);
        break;
    case 8:
        DrawTextLine(L"3D glow settings", px, top + 7.0f, right - 14.0f, top + 31.0f, Theme::Text, titleFont_);
        ColorRow(L"Glow colour", visualGlowColor_, px, top + 38.0f, pw);
        Slider(L"Thickness", visualGlowThickness_, 0.5f, 6.0f, true,
            px, top + 78.0f, pw, 42);
        break;
    }
    drawingVisualPopup_ = false;
}

void App::DrawVisualsCard(float x, float y, float w, float h)
{
    Card(x, y, w, h, L"Visuals");

    const float left = x + 12.0f;
    const float half = (w - 38.0f) * 0.5f;
    const float right = left + half + 14.0f;

    DrawVisualFeatureRow(L"Health bar", espHealthBar_, 2, left, y + 52.0f, half);
    DrawVisualFeatureRow(L"Name", espName_, 3, left, y + 83.0f, half);
    DrawVisualFeatureRow(L"Distance", espDistance_, 4, left, y + 114.0f, half);
    DrawVisualFeatureRow(L"Weapon", espWeapon_, 5, left, y + 145.0f, half);

    bool boxEnabled = boxStyle_ != 0;
    DrawVisualFeatureRow(L"Box", boxEnabled, 1, right, y + 52.0f, half);
    if (!boxEnabled && boxStyle_ != 0) boxStyle_ = 0;
    else if (boxEnabled && boxStyle_ == 0) boxStyle_ = 2;

    DrawVisualFeatureRow(L"Damage text", espDamage_, 7, right, y + 83.0f, half);
    DrawVisualFeatureRow(L"Snap line", espSnapline_, 6, right, y + 114.0f, half);
    DrawVisualFeatureRow(L"3D glow", visualGlow_, 8, right, y + 145.0f, half);

    Divider(x + 12.0f, y + 190.0f, w - 24.0f);
    DrawTextLine(L"Each visual has its own settings button.",
        x + 12.0f, y + 204.0f, x + w - 12.0f, y + 235.0f, Theme::Muted2, smallFont_);

    DrawVisualFeaturePopup(x, y, w);
}

void App::DrawParticles(float width, float height)
{
    if (!particlesEnabled_)
        return;

    const float time = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
    const int count = std::clamp(static_cast<int>(std::round(particleAmount_)), 8, 70);
    const float contentLeft = UI::SidebarWidth + 10.0f;
    const float contentTop = 6.0f;
    const float contentWidth = MaxValue(1.0f, width - contentLeft - 8.0f);
    const float contentHeight = MaxValue(1.0f, height - contentTop - 8.0f);

    for (int index = 0; index < count; ++index)
    {
        const float seed = static_cast<float>(index + 1);
        const float baseX = std::fmod(seed * 91.73f + 27.0f, contentWidth);
        const float speed = particleSpeed_ * (0.42f + std::fmod(seed * 0.173f, 0.72f));
        const float travel = std::fmod(time * speed + seed * 31.0f, contentHeight + 24.0f);
        const float x = contentLeft + baseX + std::sin(time * 0.36f + seed * 1.7f) * 8.0f;
        const float y = contentTop + contentHeight + 12.0f - travel;
        const float radius = 0.75f + std::fmod(seed * 0.39f, 1.35f);
        const float alpha = 0.16f + std::fmod(seed * 0.061f, 0.24f);

        SetBrush(MakeColor(0xFFFFFF, alpha));
        target_->FillEllipse(
            D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
            brush_);
    }
}

void App::DrawSidebar(float height)
{
    FillRect(0.0f, 0.0f, UI::SidebarWidth, height, Theme::Sidebar);
    DrawLine(UI::SidebarWidth - 1.0f, 0.0f, UI::SidebarWidth - 1.0f, height,
        Theme::SidebarBorder, 1.0f);

    constexpr float buttonSize = 42.0f;
    const float buttonLeft = (UI::SidebarWidth - buttonSize) * 0.5f;
    const float logoButton = 12.0f;
    const float settingsButton = height - buttonSize - 16.0f;
    const float saveButton = settingsButton - buttonSize - 8.0f;

    const bool settingsHover = Hover(buttonLeft, settingsButton,
        buttonLeft + buttonSize, settingsButton + buttonSize);
    const bool saveHover = Hover(buttonLeft, saveButton,
        buttonLeft + buttonSize, saveButton + buttonSize);

    DrawTriangleLogo(buttonLeft + 8.5f, logoButton + 8.0f);

    if (settingsHover || settingsOpen_)
        FillRound(buttonLeft, settingsButton, buttonLeft + buttonSize, settingsButton + buttonSize,
            9.0f, settingsOpen_ ? AccentSoft() : WithAlpha(Theme::RowHover, 0.55f));

    DrawIcon(Icon::Gear,
        buttonLeft + 7.0f, settingsButton + 7.0f,
        buttonLeft + buttonSize - 7.0f, settingsButton + buttonSize - 7.0f,
        settingsOpen_ ? AccentBright() : settingsHover ? Theme::Text : Theme::Muted);

    if (saveHover || configOpen_)
        FillRound(buttonLeft, saveButton, buttonLeft + buttonSize, saveButton + buttonSize,
            9.0f, configOpen_ ? AccentSoft() : WithAlpha(Theme::RowHover, 0.55f));

    DrawIcon(Icon::Save,
        buttonLeft + 7.0f, saveButton + 7.0f,
        buttonLeft + buttonSize - 7.0f, saveButton + buttonSize - 7.0f,
        configOpen_ ? AccentBright() : saveHover ? Theme::Text : Theme::Muted);
}

void App::DrawPreviewToggle(float width)
{
    constexpr float size = 26.0f;
    const float left = width - size - 8.0f;
    const float top = 8.0f;
    const bool hovered = Hover(left, top, left + size, top + size);

    if (hovered || !previewVisible_)
    {
        FillRound(left, top, left + size, top + size, 5.0f,
            !previewVisible_ ? AccentSoft(0.34f) : WithAlpha(Theme::RowHover, 0.58f));
    }

    DrawIcon(Icon::Eye,
        left + 4.0f, top + 4.0f,
        left + size - 4.0f, top + size - 4.0f,
        previewVisible_ ? (hovered ? Theme::Text : Theme::Muted) : AccentBright());
}

void App::DrawCards()
{
    const float margin = UI::SidebarWidth + 18.0f;
    const float gap = 18.0f;
    const float leftWidth = 330.0f;
    const float rightX = margin + leftWidth + gap;
    const float rightWidth = static_cast<float>(UI::Width) - rightX - margin;

    DrawAimbotCard(margin, 18.0f, leftWidth, 428.0f);

    // The old visual preview card was removed. The visuals controls now use
    // the complete right column, keeping both columns balanced and clean.
    DrawVisualsCard(rightX, 18.0f, rightWidth, 428.0f);
    DrawAimbotPopup(margin, 18.0f, leftWidth);
}

void App::DrawSettingsPage()
{
    const float x = UI::SidebarWidth + 24.0f;
    const float y = 20.0f;
    const float w = static_cast<float>(UI::Width) - x - 24.0f;
    const float viewTop = y + 38.0f;
    const float viewBottom = static_cast<float>(UI::Height) - 18.0f;
    const float contentHeight = 710.0f;
    const float viewHeight = viewBottom - viewTop;
    const float maxScroll = MaxValue(0.0f, contentHeight - viewHeight);
    settingsScroll_ = Clamp(settingsScroll_, 0.0f, maxScroll);

    Card(x, y, w, static_cast<float>(UI::Height) - y - 18.0f, L"Settings");

    target_->PushAxisAlignedClip(
        D2D1::RectF(x, viewTop, x + w - 10.0f, viewBottom),
        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    const float sy = viewTop - settingsScroll_;

    DrawTextLine(L"Appearance", x + 8.0f, sy + 4.0f,
        x + w - 20.0f, sy + 29.0f, Theme::Muted, titleFont_);
    ColorRow(L"Accent color", accentColor_, x + 8.0f, sy + 35.0f, w - 28.0f);
    ColorRow(L"Window tint", backgroundColor_, x + 8.0f, sy + 68.0f, w - 28.0f);
    Slider(L"Window opacity", windowOpacity_, 80.0f, 255.0f, false,
        x + 8.0f, sy + 105.0f, w - 28.0f, 13);
    Slider(L"Popup opacity", popupOpacity_, 64.0f, 255.0f, false,
        x + 8.0f, sy + 147.0f, w - 28.0f, 14);

    Divider(x + 8.0f, sy + 193.0f, w - 28.0f);
    DrawTextLine(L"Background", x + 8.0f, sy + 205.0f,
        x + w - 20.0f, sy + 230.0f, Theme::Muted, titleFont_);
    Toggle(L"Background particles", particlesEnabled_,
        x + 8.0f, sy + 238.0f, w - 28.0f);
    Slider(L"Particle speed", particleSpeed_, 4.0f, 60.0f, false,
        x + 8.0f, sy + 278.0f, w - 28.0f, 10);
    Slider(L"Particle amount", particleAmount_, 8.0f, 70.0f, false,
        x + 8.0f, sy + 320.0f, w - 28.0f, 11);

    Divider(x + 8.0f, sy + 366.0f, w - 28.0f);
    DrawTextLine(L"Preview", x + 8.0f, sy + 378.0f,
        x + w - 20.0f, sy + 403.0f, Theme::Muted, titleFont_);
    Slider(L"Character scale", previewScale_, 0.55f, 1.15f, true,
        x + 8.0f, sy + 411.0f, w - 28.0f, 12);

    Divider(x + 8.0f, sy + 457.0f, w - 28.0f);
    DrawTextLine(L"Hotkeys", x + 8.0f, sy + 469.0f,
        x + w - 20.0f, sy + 494.0f, Theme::Muted, titleFont_);
    KeybindControl(L"Open / close menu", openCloseHotkey_,
        x + 8.0f, sy + 502.0f, w - 28.0f);
    KeybindControl(L"Exit application", exitHotkey_,
        x + 8.0f, sy + 541.0f, w - 28.0f);

    DrawTextLine(L"Click a hotkey field, then press a key or key combination.",
        x + 8.0f, sy + 584.0f, x + w - 20.0f, sy + 611.0f,
        Theme::Muted2, smallFont_);
    DrawTextLine(L"The right preview uses the same particles, accent and ESP settings as the menu.",
        x + 8.0f, sy + 626.0f, x + w - 20.0f, sy + 659.0f,
        Theme::Muted2, smallFont_);

    target_->PopAxisAlignedClip();

    // Thin settings scrollbar on the far-right edge of the main window.
    const float barX = x + w - 4.0f;
    const float trackTop = viewTop + 2.0f;
    const float trackBottom = viewBottom - 2.0f;
    const float trackHeight = trackBottom - trackTop;
    const float thumbHeight = MaxValue(30.0f, trackHeight * (viewHeight / contentHeight));
    const float scrollRatio = maxScroll > 0.0f ? settingsScroll_ / maxScroll : 0.0f;
    const float thumbTop = trackTop + (trackHeight - thumbHeight) * scrollRatio;
    FillRound(barX, trackTop, barX + 2.0f, trackBottom, 1.0f, WithAlpha(Theme::Divider, 0.55f));
    FillRound(barX, thumbTop, barX + 2.0f, thumbTop + thumbHeight, 1.0f,
        WithAlpha(MakeColor(0xFFFFFF), 0.82f));
}

void App::DrawConfigPage()
{
    const float w = 324.0f;
    const float h = 330.0f;
    const float x = UI::SidebarWidth + 14.0f;
    const float y = static_cast<float>(UI::Height) - h - 18.0f;

    configPopupBounds_ = D2D1::RectF(x, y, x + w, y + h);
    configPopupBoundsValid_ = true;

    // One clean popup surface. Controls inside it do not use nested cards or borders.
    FillRound(x + 5.0f, y + 6.0f, x + w + 5.0f, y + h + 6.0f,
        1.0f, PopupColor(MakeColor(0x000000, 0.32f)));
    FillRound(x, y, x + w, y + h, 1.0f, PopupColor(MakeColor(0x07090C, 0.98f)));
    DrawRound(x, y, x + w, y + h, 1.0f, WithAlpha(accentColor_, 0.58f), 1.0f);

    DrawTextLine(L"Configurations", x + 14.0f, y + 8.0f,
        x + w - 14.0f, y + 36.0f, Theme::Text, titleFont_);

    const float inputX = x + 14.0f;
    const float inputY = y + 48.0f;
    const float inputW = w - 28.0f;
    const float inputSaveSize = 26.0f;
    const float inputSaveLeft = inputX + inputW - inputSaveSize;
    const float inputSaveTop = inputY + 3.0f;
    const bool inputSaveHover = Hover(inputSaveLeft, inputSaveTop,
        inputSaveLeft + inputSaveSize, inputSaveTop + inputSaveSize);
    const bool inputHover = Hover(inputX, inputY, inputSaveLeft - 4.0f, inputY + 32.0f);

    if (!colorPickerOpen_ && !dropdownOpen_ && mousePressed_ && !inputSaveHover)
        configNameFocused_ = inputHover;

    // Flat input: only a subtle underline, no card or border.
    DrawTextLine(configName_.empty() ? L"default_config" : configName_,
        inputX, inputY, inputSaveLeft - 8.0f, inputY + 30.0f,
        configName_.empty() ? Theme::Muted2 : Theme::Text, smallFont_);
    DrawLine(inputX, inputY + 31.0f, inputX + inputW, inputY + 31.0f,
        configNameFocused_ ? accentColor_ : Theme::Divider, 1.0f);

    if (inputSaveHover)
        FillRound(inputSaveLeft, inputSaveTop,
            inputSaveLeft + inputSaveSize, inputSaveTop + inputSaveSize,
            1.0f, AccentSoft(0.22f));
    DrawIcon(Icon::Save, inputSaveLeft, inputSaveTop,
        inputSaveLeft + inputSaveSize, inputSaveTop + inputSaveSize,
        inputSaveHover ? AccentBright() : Theme::Muted);

    DrawTextLine(L"Saved configs", inputX, y + 89.0f,
        x + w - 14.0f, y + 111.0f, Theme::Muted, smallFont_);

    const float listTop = y + 114.0f;
    const float listBottom = y + 284.0f;
    const float listHeight = listBottom - listTop;
    const float rowHeight = 28.0f;
    const int visibleRows = static_cast<int>(listHeight / rowHeight);

    if (configs_.empty())
    {
        DrawTextLine(L"No saved configs", inputX, listTop,
            inputX + inputW, listBottom, Theme::Muted2,
            smallFont_, DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else
    {
        const int first = static_cast<int>(configScroll_);
        const int last = MinValue(static_cast<int>(configs_.size()), first + visibleRows);

        for (int index = first; index < last; ++index)
        {
            const float rowTop = listTop + static_cast<float>(index - first) * rowHeight;
            const float rowBottom = rowTop + rowHeight;
            const bool selected = index == selectedConfig_;

            const float deleteRight = inputX + inputW - 2.0f;
            const float deleteLeft = deleteRight - 24.0f;
            const float loadRight = deleteLeft - 3.0f;
            const float loadLeft = loadRight - 24.0f;
            const bool loadHover = Hover(loadLeft, rowTop + 2.0f, loadRight, rowBottom - 2.0f);
            const bool deleteHover = Hover(deleteLeft, rowTop + 2.0f, deleteRight, rowBottom - 2.0f);
            const bool rowHover = Hover(inputX, rowTop, loadLeft - 4.0f, rowBottom);

            if (selected || rowHover)
            {
                FillRound(inputX, rowTop + 2.0f, inputX + inputW, rowBottom - 2.0f,
                    1.0f, selected ? AccentSoft(0.18f) : WithAlpha(Theme::RowHover, 0.42f));
            }

            if (selected)
                FillRect(inputX, rowTop + 6.0f, inputX + 2.0f, rowBottom - 6.0f, accentColor_);

            DrawIcon(Icon::Save, inputX + 7.0f, rowTop,
                inputX + 29.0f, rowBottom,
                selected ? AccentBright() : Theme::Muted2);
            DrawTextLine(configs_[index], inputX + 32.0f, rowTop,
                loadLeft - 6.0f, rowBottom,
                selected || rowHover ? Theme::Text : Theme::Muted,
                smallFont_);

            if (loadHover)
                FillRound(loadLeft, rowTop + 2.0f, loadRight, rowBottom - 2.0f,
                    1.0f, AccentSoft(0.22f));
            DrawIcon(Icon::Load, loadLeft, rowTop + 2.0f, loadRight, rowBottom - 2.0f,
                loadHover ? AccentBright() : Theme::Muted2);

            if (deleteHover)
                FillRound(deleteLeft, rowTop + 2.0f, deleteRight, rowBottom - 2.0f,
                    1.0f, MakeColor(0x431318, 0.68f));
            DrawIcon(Icon::Delete, deleteLeft, rowTop + 2.0f, deleteRight, rowBottom - 2.0f,
                deleteHover ? MakeColor(0xF06A76) : Theme::Muted2);

            if (!colorPickerOpen_ && !dropdownOpen_ && mousePressed_)
            {
                if (loadHover)
                {
                    const std::wstring name = configs_[index];
                    selectedConfig_ = index;
                    configName_ = name;
                    configNameFocused_ = false;
                    if (LoadConfig(name))
                        ShowConfigStatus(L"Loaded " + name, true);
                    else
                        ShowConfigStatus(L"Config not found", false);
                    mousePressed_ = false;
                    return;
                }

                if (deleteHover)
                {
                    const std::wstring name = configs_[index];
                    if (DeleteConfig(name))
                    {
                        ShowConfigStatus(L"Deleted " + name, true);
                        RefreshConfigs();
                        if (configs_.empty())
                        {
                            selectedConfig_ = -1;
                            configName_ = L"default_config";
                        }
                        else
                        {
                            selectedConfig_ = MinValue(index, static_cast<int>(configs_.size()) - 1);
                            configName_ = configs_[selectedConfig_];
                        }
                    }
                    else
                        ShowConfigStatus(L"Could not delete", false);
                    mousePressed_ = false;
                    return;
                }

                if (rowHover)
                {
                    selectedConfig_ = index;
                    configName_ = configs_[index];
                    configNameFocused_ = false;
                }
            }
        }

        if (configs_.size() > static_cast<std::size_t>(visibleRows))
        {
            const float maxScroll = MaxValue(1.0f,
                static_cast<float>(configs_.size() - visibleRows));
            const float trackTop = listTop + 4.0f;
            const float trackBottom = listBottom - 4.0f;
            const float thumbHeight = MaxValue(24.0f,
                (trackBottom - trackTop) *
                (static_cast<float>(visibleRows) / static_cast<float>(configs_.size())));
            const float ratio = Clamp(configScroll_ / maxScroll, 0.0f, 1.0f);
            const float thumbTop = trackTop +
                ratio * ((trackBottom - trackTop) - thumbHeight);
            FillRound(inputX + inputW - 2.0f, thumbTop,
                inputX + inputW, thumbTop + thumbHeight,
                1.0f, WithAlpha(Theme::Muted, 0.55f));
        }
    }

    // Save icon embedded in the name input.
    if (!colorPickerOpen_ && !dropdownOpen_ && inputSaveHover && mousePressed_)
    {
        const std::wstring clean = CleanConfigName(
            configName_.empty() ? L"default_config" : configName_);
        if (SaveConfig(clean))
        {
            configName_ = clean;
            RefreshConfigs();
            const auto it = std::find(configs_.begin(), configs_.end(), clean);
            if (it != configs_.end())
                selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), it));
            ShowConfigStatus(L"Saved " + clean, true);
        }
        else
            ShowConfigStatus(L"Could not save", false);
    }

    if (!configStatus_.empty())
    {
        const D2D1_COLOR_F c = configStatusOk_ ? MakeColor(0x43D17D) : MakeColor(0xE05261);
        DrawTextLine(configStatus_, inputX, y + 294.0f,
            x + w - 14.0f, y + 320.0f, c, smallFont_);
    }
}

void App::DrawGlassWindowOutline(float width, float height)
{
    const float inset = 1.0f;
    const float radius = 9.0f;

    // Subtle dark inner separation gives the glass rim definition.
    DrawRound(
        inset + 1.0f, inset + 1.0f,
        width - inset - 1.0f, height - inset - 1.0f,
        radius - 1.0f,
        MakeColor(0x05070A, 0.82f), 1.0f);

    // Main translucent glass rim.
    DrawRound(
        inset, inset,
        width - inset, height - inset,
        radius,
        MakeColor(0xFFFFFF, 0.34f), 1.0f);

    // Fine inner reflection line.
    DrawRound(
        inset + 2.0f, inset + 2.0f,
        width - inset - 2.0f, height - inset - 2.0f,
        radius - 2.0f,
        MakeColor(0xFFFFFF, 0.075f), 1.0f);

    // Long, faint top reflection.
    DrawLine(14.0f, 2.0f, width - 16.0f, 2.0f,
        MakeColor(0xFFFFFF, 0.20f), 1.0f);

    // Small specular glints similar to polished glass.
    DrawLine(15.0f, 1.8f, 48.0f, 1.8f,
        MakeColor(0xFFFFFF, 0.78f), 1.35f);
    DrawLine(width * 0.43f, 1.8f, width * 0.50f, 1.8f,
        MakeColor(0xFFFFFF, 0.48f), 1.15f);
    DrawLine(width - 112.0f, 1.8f, width - 62.0f, 1.8f,
        MakeColor(0xFFFFFF, 0.64f), 1.25f);

    // Short vertical corner reflections.
    DrawLine(1.8f, 15.0f, 1.8f, 47.0f,
        MakeColor(0xFFFFFF, 0.46f), 1.0f);
    DrawLine(width - 1.8f, 16.0f, width - 1.8f, 39.0f,
        MakeColor(0xFFFFFF, 0.20f), 1.0f);
}

void App::DrawShell(float width, float height)
{
    dropdownOverlay_ = DropdownOverlay{};

    FillRect(0.0f, 0.0f, width, height,
        BlendColor(Theme::Window, backgroundColor_, 0.035f));

    DrawParticles(width, height);
    DrawSidebar(height);
    DrawPreviewToggle(width);

    if (settingsOpen_)
        DrawSettingsPage();
    else
        DrawCards();

    if (configOpen_)
        DrawConfigPage();
    else
        configPopupBoundsValid_ = false;

    // Dedicated overlay pass: dropdowns render after every page/card so they
    // can never be covered by controls drawn later in the frame.
    DrawDropdownOverlay();

    DrawGlassWindowOutline(width, height);
    DrawColorPicker();
}

void App::SyncPreviewSettings()
{
    PreviewRenderer::Settings settings{};
    settings.characterScale = previewScale_;
    settings.particles = particlesEnabled_;
    settings.particleSpeed = particleSpeed_;
    settings.particleAmount = particleAmount_;
    settings.box = boxStyle_ != 0;
    settings.cornerBox = boxStyle_ == 2 || boxStyle_ == 4;
    settings.filled = boxStyle_ == 3 || boxStyle_ == 4;
    settings.healthStyle = healthStyle_;
    settings.healthBarWidth = healthBarWidth_;
    settings.healthBar = espHealthBar_;
    settings.name = espName_;
    settings.distance = espDistance_;
    settings.snapline = espSnapline_;
    settings.skeleton = false; // Skeleton intentionally hidden in the preview.
    settings.weapon = espWeapon_;
    settings.damage = espDamage_;
    settings.visualGlow = visualGlow_;
    settings.visualGlowThickness = visualGlowThickness_;

    auto copyColour = [](const D2D1_COLOR_F& source, float destination[4])
    {
        destination[0] = source.r;
        destination[1] = source.g;
        destination[2] = source.b;
        destination[3] = source.a;
    };

    copyColour(accentColor_, settings.accent);
    copyColour(boxColor_, settings.boxColour);
    copyColour(filledColor_, settings.filledColour);
    copyColour(healthColor_, settings.healthColour);
    copyColour(healthBackColor_, settings.healthBackColour);
    copyColour(damageColor_, settings.damageColour);
    copyColour(visualGlowColor_, settings.visualGlowColour);
    copyColour(lineColor_, settings.lineColour);
    {
        // Use the exact same background colour as DrawShell(). Do not add any
        // extra preview tint, otherwise the right panel appears different.
        const D2D1_COLOR_F previewBackground =
            BlendColor(Theme::Window, backgroundColor_, 0.035f);
        copyColour(previewBackground, settings.backgroundColour);
    }
    PreviewRenderer::SetSettings(settings);
}

void App::Render()
{
    // Window opacity is independent from popup-card opacity.
    const BYTE desiredWindowAlpha = static_cast<BYTE>(Clamp(windowOpacity_, 80.0f, 255.0f));
    if (hwnd_)
        SetLayeredWindowAttributes(hwnd_, 0, desiredWindowAlpha, LWA_ALPHA);
    if (sideHwnd_)
        SetLayeredWindowAttributes(sideHwnd_, 0, desiredWindowAlpha, LWA_ALPHA);
    if (FAILED(CreateDeviceResources()))
        return;

    RECT client{};
    GetClientRect(hwnd_, &client);
    const float width = static_cast<float>(client.right - client.left);
    const float height = static_cast<float>(client.bottom - client.top);

    if (!mouseDown_)
        activeSlider_ = -1;

    const float aimbotPopupTarget = activeAimbotPopup_ != 0 ? 1.0f : 0.0f;
    aimbotPopupAnim_ += (aimbotPopupTarget - aimbotPopupAnim_) * 0.20f;
    if (std::fabs(aimbotPopupTarget - aimbotPopupAnim_) < 0.002f)
        aimbotPopupAnim_ = aimbotPopupTarget;

    const float popupTarget = activeVisualPopup_ != 0 ? 1.0f : 0.0f;
    visualPopupAnim_ += (popupTarget - visualPopupAnim_) * 0.20f;
    if (std::fabs(popupTarget - visualPopupAnim_) < 0.002f)
        visualPopupAnim_ = popupTarget;

    if (configStatusTime_ > 0.0f)
    {
        configStatusTime_ -= 0.016f;
        if (configStatusTime_ <= 0.0f)
        {
            configStatusTime_ = 0.0f;
            configStatus_.clear();
        }
    }

    target_->BeginDraw();
    target_->SetTransform(D2D1::Matrix3x2F::Identity());
    target_->Clear(Theme::Window);

    DrawShell(width, height);
    SyncPreviewSettings();

    const HRESULT hr = target_->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
        DiscardDeviceResources();

    mousePressed_ = false;
    popupOwnsClick_ = false;
}

LRESULT CALLBACK App::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    App* app = nullptr;

    if (message == WM_NCCREATE)
    {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        app->hwnd_ = hwnd;
    }
    else
    {
        app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    return app ? app->WindowProc(message, wParam, lParam)
        : DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT App::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_MOVE:
        if (sideHwnd_)
        {
            RECT mainRect{};
            GetWindowRect(hwnd_, &mainRect);
            SetWindowPos(sideHwnd_, nullptr,
                mainRect.right + UI::WindowGap,
                mainRect.top,
                UI::SideWidth,
                UI::Height,
                SWP_NOACTIVATE | SWP_NOZORDER);
        }
        return 0;

    case WM_NCHITTEST:
    {
        const LRESULT result = DefWindowProcW(hwnd_, message, wParam, lParam);
        if (result != HTCLIENT)
            return result;

        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd_, &pt);

        RECT client{};
        GetClientRect(hwnd_, &client);

        // Keep sidebar buttons clickable. The empty upper content strip drags the window.
        constexpr float buttonSize = 42.0f;
        const float buttonLeft = (UI::SidebarWidth - buttonSize) * 0.5f;
        const float logoButton = 12.0f;
        const float settingsButton = static_cast<float>(client.bottom) - buttonSize - 16.0f;
        const float saveButton = settingsButton - buttonSize - 8.0f;
        constexpr float previewToggleSize = 26.0f;
        const float previewToggleLeft = static_cast<float>(client.right) - previewToggleSize - 8.0f;
        const float previewToggleTop = 8.0f;

        if (Hit(static_cast<float>(pt.x), static_cast<float>(pt.y),
                buttonLeft, logoButton, buttonLeft + buttonSize, logoButton + buttonSize) ||
            Hit(static_cast<float>(pt.x), static_cast<float>(pt.y),
                buttonLeft, settingsButton, buttonLeft + buttonSize, settingsButton + buttonSize) ||
            Hit(static_cast<float>(pt.x), static_cast<float>(pt.y),
                buttonLeft, saveButton, buttonLeft + buttonSize, saveButton + buttonSize) ||
            Hit(static_cast<float>(pt.x), static_cast<float>(pt.y),
                previewToggleLeft, previewToggleTop,
                previewToggleLeft + previewToggleSize, previewToggleTop + previewToggleSize))
            return HTCLIENT;

        if (pt.x >= static_cast<LONG>(UI::SidebarWidth) &&
            pt.y >= 0 && pt.y < static_cast<LONG>(UI::DragHeight))
            return HTCAPTION;
        return HTCLIENT;
    }

    case WM_MOUSEMOVE:
        mouse_.x = GET_X_LPARAM(lParam);
        mouse_.y = GET_Y_LPARAM(lParam);
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_MOUSEWHEEL:
    {
        if (settingsOpen_)
        {
            const float direction = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -42.0f : 42.0f;
            const float viewHeight = (static_cast<float>(UI::Height) - 18.0f) - (20.0f + 38.0f);
            const float maxScroll = MaxValue(0.0f, 610.0f - viewHeight);
            settingsScroll_ = Clamp(settingsScroll_ + direction, 0.0f, maxScroll);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (configOpen_ && configs_.size() > 8)
        {
            POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd_, &point);

            const float popupX = UI::SidebarWidth + 14.0f;
            const float popupY = static_cast<float>(UI::Height) - 330.0f - 18.0f;
            const float listLeft = popupX + 14.0f;
            const float listTop = popupY + 114.0f;
            const float listRight = popupX + 324.0f - 14.0f;
            const float listBottom = popupY + 284.0f;

            if (Hit(static_cast<float>(point.x), static_cast<float>(point.y),
                listLeft, listTop, listRight, listBottom))
            {
                const float direction =
                    GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -1.0f : 1.0f;
                const float maxScroll =
                    MaxValue(0.0f, static_cast<float>(configs_.size()) - 6.0f);
                configScroll_ = Clamp(configScroll_ + direction, 0.0f, maxScroll);
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        mouse_.x = GET_X_LPARAM(lParam);
        mouse_.y = GET_Y_LPARAM(lParam);
        popupOwnsClick_ = false;

        RECT client{};
        GetClientRect(hwnd_, &client);

        constexpr float buttonSize = 42.0f;
        const float buttonLeft = (UI::SidebarWidth - buttonSize) * 0.5f;
        const float logoButton = 12.0f;
        const float settingsButton = static_cast<float>(client.bottom) - buttonSize - 16.0f;
        const float saveButton = settingsButton - buttonSize - 8.0f;
        constexpr float previewToggleSize = 26.0f;
        const float previewToggleLeft = static_cast<float>(client.right) - previewToggleSize - 8.0f;
        const float previewToggleTop = 8.0f;

        const bool logoHit = Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
            buttonLeft, logoButton, buttonLeft + buttonSize, logoButton + buttonSize);
        const bool settingsHit = Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
            buttonLeft, settingsButton, buttonLeft + buttonSize, settingsButton + buttonSize);
        const bool saveHit = Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
            buttonLeft, saveButton, buttonLeft + buttonSize, saveButton + buttonSize);
        const bool previewToggleHit = Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
            previewToggleLeft, previewToggleTop,
            previewToggleLeft + previewToggleSize, previewToggleTop + previewToggleSize);

        if (!colorPickerOpen_ && previewToggleHit)
        {
            previewVisible_ = !previewVisible_;
            if (sideHwnd_)
                ShowWindow(sideHwnd_, (windowsVisible_ && previewVisible_) ? SW_SHOWNA : SW_HIDE);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (!colorPickerOpen_ && logoHit)
        {
            settingsOpen_ = false;
            configOpen_ = false;
            dropdownOpen_ = false;
            dropdownOverlay_ = DropdownOverlay{};
            keybindCapture_ = false;
            keybindCaptureTarget_ = nullptr;
            configNameFocused_ = false;
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (!colorPickerOpen_ && settingsHit)
        {
            settingsOpen_ = !settingsOpen_;
            configOpen_ = false;
            activeAimbotPopup_ = 0;
            dropdownOpen_ = false;
            dropdownOverlay_ = DropdownOverlay{};
            keybindCapture_ = false;
            keybindCaptureTarget_ = nullptr;
            configNameFocused_ = false;
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (!colorPickerOpen_ && saveHit)
        {
            configOpen_ = !configOpen_;
            activeAimbotPopup_ = 0;
            configName_ = configName_.empty() ? L"default_config" : configName_;
            dropdownOpen_ = false;
            dropdownOverlay_ = DropdownOverlay{};
            keybindCapture_ = false;
            keybindCaptureTarget_ = nullptr;
            configNameFocused_ = false;
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (!colorPickerOpen_ && configOpen_ && configPopupBoundsValid_)
        {
            const bool insideConfigPopup = Hit(
                static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
                configPopupBounds_.left, configPopupBounds_.top,
                configPopupBounds_.right, configPopupBounds_.bottom);
            if (!insideConfigPopup)
            {
                configOpen_ = false;
                configPopupBoundsValid_ = false;
                configNameFocused_ = false;
                popupOwnsClick_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0; // close only; never click through to controls behind
            }
            popupOwnsClick_ = true;
        }

        if (!colorPickerOpen_ && activeAimbotPopup_ != 0 && aimbotPopupBoundsValid_)
        {
            const bool insideAimbotPopup = Hit(
                static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
                aimbotPopupBounds_.left, aimbotPopupBounds_.top,
                aimbotPopupBounds_.right, aimbotPopupBounds_.bottom);
            if (!insideAimbotPopup)
            {
                activeAimbotPopup_ = 0;
                aimbotPopupBoundsValid_ = false;
                popupOwnsClick_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }
            popupOwnsClick_ = true;
        }

        if (!colorPickerOpen_ && activeVisualPopup_ != 0 && visualPopupBoundsValid_)
        {
            const float mouseX = static_cast<float>(mouse_.x);
            const float mouseY = static_cast<float>(mouse_.y);

            const bool insideVisualPopup = Hit(
                mouseX, mouseY,
                visualPopupBounds_.left, visualPopupBounds_.top,
                visualPopupBounds_.right, visualPopupBounds_.bottom);

            // A dropdown opened by this popup may extend beyond the popup card.
            // Treat its button and item list as part of the active popup so the
            // first click selects an item instead of closing the parent card.
            bool insidePopupDropdown = false;
            if (dropdownOpen_ && dropdownOverlay_.valid)
            {
                const float dropdownBottom = dropdownOverlay_.top +
                    dropdownOverlay_.itemHeight *
                    static_cast<float>(dropdownOverlay_.items.size());

                insidePopupDropdown = Hit(
                    mouseX, mouseY,
                    dropdownOverlay_.buttonLeft, dropdownOverlay_.buttonTop,
                    dropdownOverlay_.buttonRight, dropdownOverlay_.buttonBottom) ||
                    Hit(
                        mouseX, mouseY,
                        dropdownOverlay_.left, dropdownOverlay_.top,
                        dropdownOverlay_.right, dropdownBottom);
            }

            if (!insideVisualPopup && !insidePopupDropdown)
            {
                activeVisualPopup_ = 0;
                visualPopupBoundsValid_ = false;
                dropdownOpen_ = false;
                dropdownOverlay_ = DropdownOverlay{};
                popupOwnsClick_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0; // close only; never activate a control underneath
            }
            popupOwnsClick_ = true;
        }

        if (!colorPickerOpen_ && dropdownOpen_ && dropdownOverlay_.valid)
        {
            const float popupBottom = dropdownOverlay_.top +
                dropdownOverlay_.itemHeight * static_cast<float>(dropdownOverlay_.items.size());

            const bool insideButton = Hit(
                static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
                dropdownOverlay_.buttonLeft, dropdownOverlay_.buttonTop,
                dropdownOverlay_.buttonRight, dropdownOverlay_.buttonBottom);

            const bool insidePopup = Hit(
                static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
                dropdownOverlay_.left, dropdownOverlay_.top,
                dropdownOverlay_.right, popupBottom);

            if (!insideButton && !insidePopup)
            {
                dropdownOpen_ = false;
                dropdownOverlay_ = DropdownOverlay{};
                popupOwnsClick_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }
            popupOwnsClick_ = true;
        }

        SetCapture(hwnd_);
        mouseDown_ = true;
        mousePressed_ = true;
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;
    }

    case WM_LBUTTONUP:
        mouseDown_ = false;

        // End color-palette/hue dragging immediately. Without this reset,
        // the next click on Apply could still be treated as a palette drag.
        // Because Apply is below the palette, the value was clamped to 0,
        // which changed the selected color to black.
        activeColorArea_ = -1;

        ReleaseCapture();
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_CHAR:
        if (configOpen_ && configNameFocused_)
        {
            const wchar_t character = static_cast<wchar_t>(wParam);

            if (character == L'\b')
            {
                if (!configName_.empty())
                    configName_.pop_back();
            }
            else if (character >= 32 && configName_.size() < 36 &&
                (std::iswalnum(character) || character == L' ' ||
                    character == L'-' || character == L'_'))
            {
                configName_.push_back(character);
            }

            InvalidateRect(hwnd_, nullptr, FALSE);
        }
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        const int virtualKey = static_cast<int>(wParam);
        const bool isRepeat = (lParam & (1LL << 30)) != 0;

        if (configNameFocused_)
        {
            if (virtualKey == VK_ESCAPE)
            {
                configNameFocused_ = false;
            }
            else if (virtualKey == VK_RETURN)
            {
                configNameFocused_ = false;
                const std::wstring clean = CleanConfigName(configName_);
                if (clean.empty())
                {
                    ShowConfigStatus(L"Enter a valid config name.", false);
                }
                else if (SaveConfig(clean))
                {
                    configName_ = clean;
                    RefreshConfigs();
                    const auto iterator = std::find(configs_.begin(), configs_.end(), clean);
                    if (iterator != configs_.end())
                        selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), iterator));
                    ShowConfigStatus(L"Configuration saved.", true);
                }
                else
                {
                    ShowConfigStatus(L"Could not save the configuration.", false);
                }
            }

            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (keybindCapture_)
        {
            if (virtualKey == VK_ESCAPE)
            {
                keybindCapture_ = false;
                keybindCaptureTarget_ = nullptr;
            }
            else if (virtualKey == VK_BACK)
            {
                if (keybindCaptureTarget_)
                    *keybindCaptureTarget_ = 0;
                keybindCapture_ = false;
                keybindCaptureTarget_ = nullptr;
                UpdateRegisteredHotkey();
            }
            else if (!IsModifierKey(virtualKey))
            {
                if (keybindCaptureTarget_)
                    *keybindCaptureTarget_ = EncodeHotkey(virtualKey);
                keybindCapture_ = false;
                keybindCaptureTarget_ = nullptr;
                UpdateRegisteredHotkey();
            }

            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (virtualKey == VK_ESCAPE)
        {
            if (colorPickerOpen_ && pickerTarget_)
            {
                *pickerTarget_ = pickerOriginal_;
                colorPickerOpen_ = false;
                pickerTarget_ = nullptr;
                activeColorArea_ = -1;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            else
            {
                PostMessageW(hwnd_, WM_CLOSE, 0, 0);
            }
            return 0;
        }

        // Registered hotkeys work even while another window is focused.
        // Ctrl, Alt and Shift combinations are supported, for example Ctrl+Alt+X.
        if (!hotkeyRegistered_ && !isRepeat && MatchesHotkey(aimbotKeybind_, virtualKey))
        {
            aimEnabled_ = !aimEnabled_;
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        return 0;
    }

    case WM_HOTKEY:
        if (wParam == 1)
        {
            aimEnabled_ = !aimEnabled_;
            InvalidateRect(hwnd_, nullptr, FALSE);
        }
        else if (wParam == 2)
        {
            windowsVisible_ = !windowsVisible_;
            ShowWindow(hwnd_, windowsVisible_ ? SW_SHOW : SW_HIDE);
            ShowWindow(sideHwnd_, (windowsVisible_ && previewVisible_) ? SW_SHOWNA : SW_HIDE);
            if (windowsVisible_) SetForegroundWindow(hwnd_);
        }
        else if (wParam == 3)
        {
            PostMessageW(hwnd_, WM_CLOSE, 0, 0);
        }
        return 0;

    case WM_SIZE:
        if (target_)
            target_->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd_, &ps);
        Render();
        EndPaint(hwnd_, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_SETCURSOR:
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        return TRUE;

    case WM_DESTROY:
        KillTimer(hwnd_, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd_, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    App app;
    if (!app.Initialize(instance))
    {
        MessageBoxW(nullptr, L"Failed to initialize Direct2D UI.", L"Error", MB_OK | MB_ICONERROR);
        app.Shutdown();
        return 1;
    }

    const int result = app.Run();
    app.Shutdown();
    return result;
}