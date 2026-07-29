#pragma once

// Shared MaotawUI types, theme, layout, and settings.
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <algorithm>
#include <array>
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

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

template <typename T>
inline void SafeRelease(T*& value)
{
    if (value)
    {
        value->Release();
        value = nullptr;
    }
}

inline float Clamp(float value, float minimum, float maximum)
{
    return std::max(minimum, std::min(value, maximum));
}

inline bool Hit(float x, float y, float left, float top, float right, float bottom)
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

inline D2D1_COLOR_F MakeColor(std::uint32_t rgbValue, float alpha = 1.0f)
{
    return D2D1::ColorF(
        static_cast<float>((rgbValue >> 16) & 0xFF) / 255.0f,
        static_cast<float>((rgbValue >> 8) & 0xFF) / 255.0f,
        static_cast<float>(rgbValue & 0xFF) / 255.0f,
        alpha);
}

inline D2D1_COLOR_F WithAlpha(const D2D1_COLOR_F& color, float alpha)
{
    return D2D1::ColorF(color.r, color.g, color.b, alpha);
}

inline D2D1_COLOR_F HsvToColor(float hue, float saturation, float value)
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

inline void ColorToHsv(const D2D1_COLOR_F& color, float& hue, float& saturation, float& value)
{
    const float maximum = std::max({ color.r, color.g, color.b });
    const float minimum = std::min({ color.r, color.g, color.b });
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

namespace Layout
{
    constexpr int Width = 760;
    constexpr int Height = 470;

    constexpr float Sidebar = 72.0f;
    constexpr float SplitX = 418.0f;
    constexpr float DragStrip = 14.0f;

    constexpr float LeftX = 101.0f;
    constexpr float LeftRight = 397.0f;
    constexpr float RightX = 438.0f;
    constexpr float RightRight = 736.0f;
}

namespace Theme
{
    inline const D2D1_COLOR_F Window = MakeColor(0x090A0D);
    inline const D2D1_COLOR_F Sidebar = MakeColor(0x0B0C10);
    inline const D2D1_COLOR_F Row = MakeColor(0x0E1014);
    inline const D2D1_COLOR_F RowHover = MakeColor(0x13161B);
    inline const D2D1_COLOR_F Popup = MakeColor(0x0B0C0F);
    inline const D2D1_COLOR_F PopupHover = MakeColor(0x15171B);

    inline const D2D1_COLOR_F Line = MakeColor(0x171A20);
    inline const D2D1_COLOR_F LineSoft = MakeColor(0x12151A);
    inline const D2D1_COLOR_F Track = MakeColor(0x20242B);

    inline const D2D1_COLOR_F Text = MakeColor(0xD4D7DC);
    inline const D2D1_COLOR_F SubText = MakeColor(0x777D87);
    inline const D2D1_COLOR_F Muted = MakeColor(0x484E58);
    inline const D2D1_COLOR_F White = MakeColor(0xF4F5F7);

    inline const D2D1_COLOR_F Green = MakeColor(0x52D628);
    inline const D2D1_COLOR_F Red = MakeColor(0xE21E2A);
    inline const D2D1_COLOR_F Light = MakeColor(0xD5D8DE);
    inline const D2D1_COLOR_F Amber = MakeColor(0x8D5700);
    inline const D2D1_COLOR_F Blue = MakeColor(0x91A5D4);
    inline const D2D1_COLOR_F Success = MakeColor(0x55D98A);
    inline const D2D1_COLOR_F Danger = MakeColor(0xE45A67);
    inline const D2D1_COLOR_F Panel = MakeColor(0x0C0E12);
}

namespace Glyph
{
    constexpr const wchar_t* Target = L"\xF272";
    constexpr const wchar_t* Eye = L"\xE890";
    constexpr const wchar_t* Settings = L"\xE713";
    constexpr const wchar_t* Folder = L"\xE8B7";
    constexpr const wchar_t* Cloud = L"\xE753";
    constexpr const wchar_t* Chevron = L"\xE70D";
    constexpr const wchar_t* Profile = L"\xE77B";
    constexpr const wchar_t* Save = L"\xE74E";
    constexpr const wchar_t* Load = L"\xE72C";
    constexpr const wchar_t* Delete = L"\xE74D";
    constexpr const wchar_t* Add = L"\xE710";
    constexpr const wchar_t* Check = L"\xE73E";
    constexpr const wchar_t* Close = L"\xE711";
    constexpr const wchar_t* Info = L"\xE946";
    constexpr const wchar_t* Sparkle = L"\xE945";
    constexpr const wchar_t* Local = L"\xE77B";
    constexpr const wchar_t* Enemy = L"\xE7EF";
    constexpr const wchar_t* Box = L"\xE739";
}

struct VisualProfile
{
    bool enabled = true;
    bool box = true;
    bool filledBox = false;
    bool name = true;
    bool healthBar = true;
    bool armorBar = false;
    bool skeleton = false;
    bool weapon = true;
    bool distance = true;
    bool snaplines = false;
    bool glow = false;
    bool chams = false;
    bool damageText = false;
    bool offscreenArrows = false;

    int boxStyle = 0;
    int healthPosition = 0;
    int armorPosition = 1;
    int healthStyle = 1;
    int armorStyle = 1;
    int snaplinePosition = 0;
    int namePosition = 0;

    float boxThickness = 1.0f;
    float fillOpacity = 18.0f;
    float skeletonThickness = 1.0f;
    float glowStrength = 5.0f;
    float arrowSize = 12.0f;
    float maxDistance = 250.0f;

    D2D1_COLOR_F boxColor = MakeColor(0xD5D8DE);
    D2D1_COLOR_F boxHiddenColor = MakeColor(0xE45A67);
    D2D1_COLOR_F fillColor = MakeColor(0x91A5D4);
    D2D1_COLOR_F nameColor = MakeColor(0xF4F5F7);
    D2D1_COLOR_F weaponColor = MakeColor(0xF4F5F7);
    D2D1_COLOR_F distanceColor = MakeColor(0xAEB4BE);
    D2D1_COLOR_F snaplineColor = MakeColor(0xD5D8DE);
    D2D1_COLOR_F damageTextColor = MakeColor(0xFFFFFF);
    D2D1_COLOR_F healthHighColor = MakeColor(0x55D98A);
    D2D1_COLOR_F healthLowColor = MakeColor(0xE45A67);
    D2D1_COLOR_F healthBackColor = MakeColor(0x20242B);
    D2D1_COLOR_F armorColor = MakeColor(0x91A5D4);
    D2D1_COLOR_F armorBackColor = MakeColor(0x20242B);
    D2D1_COLOR_F skeletonColor = MakeColor(0xF4F5F7);
    D2D1_COLOR_F glowColor = MakeColor(0x91A5D4);
    D2D1_COLOR_F chamsVisibleColor = MakeColor(0x55D98A);
    D2D1_COLOR_F chamsHiddenColor = MakeColor(0xE45A67);
    D2D1_COLOR_F arrowColor = MakeColor(0xF4F5F7);
};

inline VisualProfile MakeDefaultVisualProfile()
{
    return VisualProfile{};
}

struct Settings
{
    bool enableScroller = true;
    bool connectedParticles = true;
    bool windowBorder = false;

    float particleSpeed = 22.0f;
    float particleCount = 26.0f;
    float connectionDistance = 92.0f;
    float particleOpacity = 48.0f;
    float windowOpacity = 100.0f;
    float windowBorderOpacity = 100.0f;
    float dpiScale = 134.7f;

    D2D1_COLOR_F pickerColor = Theme::Amber;
    D2D1_COLOR_F accentColor = Theme::White;
    D2D1_COLOR_F particleColor = Theme::White;

    // Both profiles start from one canonical default, then remain independent.
    VisualProfile local = MakeDefaultVisualProfile();
    VisualProfile enemy = MakeDefaultVisualProfile();
};
