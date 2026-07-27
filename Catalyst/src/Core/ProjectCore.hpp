#pragma once

// ============================================================================
// main.cpp
// Win32 + Direct3D 11 + Direct2D 1.1 + DirectWrite.
// No external libraries.
//
// Left: visuals controls using the supplied dark Direct2D UI style.
// Right: embedded 3D character rendered with Direct3D 11, with a clean
// transparent preview area, Visuals-controlled 3D outline and animated health overlays.
//
// This is a visual UI template only. It does not read or modify another
// process and contains no game integration.
//
// Visual Studio libraries:
//   d3d11.lib
//   dxgi.lib
//   d3dcompiler.lib
//   d2d1.lib
//   dwrite.lib
//
// Build from x64 Native Tools Command Prompt:
// cl /std:c++17 /EHsc /DUNICODE /D_UNICODE main.cpp /link ^
// d3d11.lib dxgi.lib d3dcompiler.lib d2d1.lib dwrite.lib user32.lib gdi32.lib
// ============================================================================

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace
{
    constexpr int WINDOW_W = 910;
    constexpr int WINDOW_H = 600;

    constexpr float TOP_H = 70.0f;
    constexpr float PAD = 18.0f;
    constexpr float GAP = 18.0f;
    constexpr float LEFT_W = 390.0f;

    struct Colours
    {
        D2D1_COLOR_F background = D2D1::ColorF(0x01 / 255.0f, 0x02 / 255.0f, 0x03 / 255.0f, 1.0f);
        D2D1_COLOR_F top = D2D1::ColorF(0x02 / 255.0f, 0x03 / 255.0f, 0x04 / 255.0f, 1.0f);
        D2D1_COLOR_F panel = D2D1::ColorF(0x07 / 255.0f, 0x09 / 255.0f, 0x0C / 255.0f, 0.96f);
        D2D1_COLOR_F row = D2D1::ColorF(0x09 / 255.0f, 0x0B / 255.0f, 0x0E / 255.0f, 1.0f);
        D2D1_COLOR_F hover = D2D1::ColorF(0x11 / 255.0f, 0x14 / 255.0f, 0x18 / 255.0f, 0.72f);
        D2D1_COLOR_F border = D2D1::ColorF(0x11 / 255.0f, 0x13 / 255.0f, 0x17 / 255.0f, 1.0f);
        D2D1_COLOR_F text = D2D1::ColorF(0xE6 / 255.0f, 0xE6 / 255.0f, 0xE6 / 255.0f, 1.0f);
        D2D1_COLOR_F muted = D2D1::ColorF(0x7D / 255.0f, 0x81 / 255.0f, 0x89 / 255.0f, 1.0f);
        D2D1_COLOR_F track = D2D1::ColorF(0x17 / 255.0f, 0x19 / 255.0f, 0x1D / 255.0f, 1.0f);
        D2D1_COLOR_F white = D2D1::ColorF(0xF4 / 255.0f, 0xF5 / 255.0f, 0xF7 / 255.0f, 1.0f);
    } ui;

    D2D1_COLOR_F MakeColour(std::uint32_t rgb, float alpha = 1.0f)
    {
        return D2D1::ColorF(
            static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
            static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
            static_cast<float>(rgb & 0xFF) / 255.0f,
            alpha);
    }

    D2D1_COLOR_F WithAlpha(const D2D1_COLOR_F& c, float alpha)
    {
        return D2D1::ColorF(c.r, c.g, c.b, alpha);
    }

    float Clamp(float v, float minimum, float maximum)
    {
        return std::max(minimum, std::min(v, maximum));
    }

    bool Hit(float x, float y, const D2D1_RECT_F& r)
    {
        return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
    }

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
    };

    struct alignas(16) ConstantBuffer
    {
        XMFLOAT4X4 worldViewProjection;
        XMFLOAT4X4 world;
        XMFLOAT4 colour;

        // Packed into one 16-byte HLSL constant-buffer register.
        float outlinePass = 0.0f;
        float outlineWidthPixels = 0.0f;
        float viewportWidth = 1.0f;
        float viewportHeight = 1.0f;
    };

    static_assert(sizeof(ConstantBuffer) == 160,
        "ConstantBuffer must exactly match the HLSL cbuffer layout.");
    static_assert((sizeof(ConstantBuffer) % 16) == 0,
        "D3D11 constant buffers must be a multiple of 16 bytes.");

    struct VisualSettings
    {
        bool box = true;
        bool healthBar = true;
        bool healthText = true;
        bool damageText = true;
        bool name = true;
        bool distance = true;
        bool skeleton = false;
        bool snapline = false;
        bool soundWalk = false;
        bool soundMarker = false;
        // This Visuals-page option controls the real 3D inverted-hull outline.
        bool visualGlow = true;
        bool rotateModel = true;

        // Per-visual style settings opened from the small gear beside each paint icon.
        int boxStyle = 1;       // 0 full, 1 corners, 2 full + corners, 3 filled, 4 filled + corners
        int healthBarStyle = 2; // 0 classic, 1 thin, 2 segmented, 3 blocks, 4 split, 5 bottom
        int snaplineOrigin = 0; // 0 bottom, 1 centre, 2 top
        int soundWalkAnimationStyle = 1; // 0 smooth, 1 cinematic, 2 fast, 3 minimal

        float boxThickness = 1.6f;
        float boxFillAlpha = 0.24f;
        float healthBarThickness = 5.2f;
        float healthOutlineThickness = 1.0f;
        float snaplineThickness = 1.2f;
        float skeletonThickness = 1.2f;
        float visualGlowThickness = 2.0f;
        float soundWalkSpeed = 0.72f;
        float soundWalkExpansion = 1.45f;
        float soundWalkThickness = 1.6f;
        float modelScale = 0.94f;

        D2D1_COLOR_F boxColour = MakeColour(0xE9EDF5);
        D2D1_COLOR_F boxFillColour = MakeColour(0xE9EDF5);
        D2D1_COLOR_F healthColour = MakeColour(0x42D66B);
        D2D1_COLOR_F healthBackgroundColour = MakeColour(0x15181D);
        D2D1_COLOR_F healthOutlineColour = MakeColour(0x050607);
        D2D1_COLOR_F damageTextColour = MakeColour(0xFFFFFF);
        D2D1_COLOR_F skeletonColour = MakeColour(0x65B8FF);
        D2D1_COLOR_F snaplineColour = MakeColour(0xFF8E44);
        D2D1_COLOR_F soundWalkColour = MakeColour(0xB8E7FF);
        D2D1_COLOR_F soundMarkerColour = MakeColour(0xF6C160);
        D2D1_COLOR_F visualGlowColour = MakeColour(0xFFFFFF);
        D2D1_COLOR_F modelColour = MakeColour(0xF8D6D6);
        D2D1_COLOR_F accentColour = MakeColour(0xFFFFFF);
    };

    enum class SliderId
    {
        None,
        BoxThickness,
        BoxFillAlpha,
        HealthBarThickness,
        HealthOutlineThickness,
        SnaplineThickness,
        SkeletonThickness,
        VisualGlowThickness,
        SoundWalkSpeed,
        SoundWalkExpansion,
        SoundWalkThickness,
        ModelScale,
        ParticleSpeed,
        ParticleAmount
    };

    enum class VisualPopup
    {
        None,
        Box,
        HealthBar,
        Snapline,
        Skeleton,
        SoundWalk,
        ModelOutline
    };

    enum class PopupDropdown
    {
        None,
        BoxStyle,
        HealthStyle,
        SnaplineOrigin,
        SoundWalkAnimation
    };

    struct Slider
    {
        SliderId id = SliderId::None;
        D2D1_RECT_F hit{};
        float* value = nullptr;
        float minimum = 0.0f;
        float maximum = 1.0f;
    };

    struct App
    {
        HWND hwnd = nullptr;
        UINT width = WINDOW_W;
        UINT height = WINDOW_H;

        // D3D
        ComPtr<ID3D11Device> d3dDevice;
        ComPtr<ID3D11DeviceContext> d3dContext;
        ComPtr<IDXGISwapChain1> swapChain;
        ComPtr<ID3D11RenderTargetView> renderTargetView;
        ComPtr<ID3D11Texture2D> depthTexture;
        ComPtr<ID3D11DepthStencilView> depthView;
        ComPtr<ID3D11VertexShader> vertexShader;
        ComPtr<ID3D11PixelShader> pixelShader;
        ComPtr<ID3D11InputLayout> inputLayout;
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
        ComPtr<ID3D11Buffer> constantBuffer;
        UINT embeddedIndexCount = 0;
        ComPtr<ID3D11RasterizerState> rasterizerState;
        ComPtr<ID3D11RasterizerState> outlineRasterizerState;
        ComPtr<ID3D11BlendState> alphaBlend;
        ComPtr<ID3D11BlendState> colourWriteDisabledBlend;
        ComPtr<ID3D11DepthStencilState> depthState;
        ComPtr<ID3D11DepthStencilState> outlineDepthState;

        // D2D/DWrite
        ComPtr<ID2D1Factory1> d2dFactory;
        ComPtr<ID2D1Device> d2dDevice;
        ComPtr<ID2D1DeviceContext> d2dContext;
        ComPtr<ID2D1Bitmap1> d2dTarget;
        ComPtr<ID2D1SolidColorBrush> brush;
        ComPtr<IDWriteFactory> writeFactory;
        ComPtr<IDWriteTextFormat> text12;
        ComPtr<IDWriteTextFormat> text13;
        ComPtr<IDWriteTextFormat> text14;
        ComPtr<IDWriteTextFormat> text16Bold;
        ComPtr<IDWriteTextFormat> text19Bold;
        ComPtr<IDWriteTextFormat> text27Bold;
        ComPtr<IDWriteTextFormat> iconFont;

        VisualSettings settings;

        bool settingsOpen = false;
        bool particlesEnabled = true;
        float particleSpeed = 22.0f;
        float particleAmount = 34.0f;

        // Automatic preview damage animation. This is intentionally not user controlled.
        float previewHealth = 100.0f;
        float previewHealthTarget = 100.0f;
        float healthAnimationTimer = 0.70f;
        int healthDamageStep = 0;
        float damageTextTimer = 0.0f;
        int damageTextValue = 0;

        // Small per-visual settings cards opened by the gear beside each paint icon.
        VisualPopup visualPopup = VisualPopup::None;
        D2D1_RECT_F visualPopupRect{};
        float visualPopupAnchorY = 0.0f;
        float visualPopupX = 0.0f;
        float visualPopupY = 0.0f;
        bool visualPopupPositioned = false;
        bool draggingVisualPopup = false;
        float visualPopupDragOffsetX = 0.0f;
        float visualPopupDragOffsetY = 0.0f;

        // Reused by the style/origin dropdowns inside the small settings cards.
        bool healthStyleDropdownOpen = false;
        PopupDropdown popupDropdown = PopupDropdown::None;
        D2D1_RECT_F healthStyleButtonRect{};
        D2D1_RECT_F healthStylePopupRect{};

        std::wstring saveStatus;
        float saveStatusTimer = 0.0f;

        // Named configuration manager opened by the top save icon.
        bool configOpen = false;
        std::wstring configName;
        bool configNameFocused = false;
        std::vector<std::wstring> configs;
        int selectedConfig = -1;
        float configScroll = 0.0f;
        std::wstring configStatus;
        bool configStatusOk = true;
        float configStatusTimer = 0.0f;
        D2D1_RECT_F configPanelRect{};
        D2D1_RECT_F configInputRect{};

        // Rebuilt every frame and used to show a hand cursor over controls.
        std::vector<D2D1_RECT_F> interactiveRects;

        POINT mouse{ -1000, -1000 };
        bool mouseDown = false;
        bool clicked = false;
        bool clickConsumed = false;

        SliderId activeSlider = SliderId::None;
        std::vector<Slider> sliders;

        bool colourPickerOpen = false;
        D2D1_COLOR_F* colourTarget = nullptr;
        D2D1_COLOR_F originalColour{};
        D2D1_COLOR_F workingColour{};
        float pickerHue = 0.0f;
        float pickerSaturation = 0.0f;
        float pickerValue = 1.0f;
        int activePickerArea = -1;

        bool draggingModel = false;
        float modelYaw = -0.45f;
        float previousMouseX = 0.0f;

        D2D1_RECT_F previewRect{};
    } app;

    D2D1_RECT_F Rect(float l, float t, float r, float b)
    {
        return D2D1::RectF(l, t, r, b);
    }

    void RegisterInteractive(const D2D1_RECT_F& rect)
    {
        app.interactiveRects.push_back(rect);
    }

    bool IsInteractiveAt(float x, float y)
    {
        for (const D2D1_RECT_F& rect : app.interactiveRects)
        {
            if (Hit(x, y, rect))
                return true;
        }
        return false;
    }

    void UpdatePointerCursor()
    {
        const float x = static_cast<float>(app.mouse.x);
        const float y = static_cast<float>(app.mouse.y);

        if (app.configOpen && Hit(x, y, app.configInputRect))
        {
            SetCursor(LoadCursorW(nullptr, IDC_IBEAM));
            return;
        }

        SetCursor(LoadCursorW(nullptr, IsInteractiveAt(x, y) ? IDC_HAND : IDC_ARROW));
    }

    // ------------------------------------------------------------------------
    // Colour conversion
    // ------------------------------------------------------------------------

    D2D1_COLOR_F HsvToColour(float hue, float saturation, float value)
    {
        hue -= std::floor(hue);
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
        default:return D2D1::ColorF(value, p, q, 1.0f);
        }
    }

    void ColourToHsv(const D2D1_COLOR_F& colour, float& hue, float& saturation, float& value)
    {
        const float maximum = std::max({ colour.r, colour.g, colour.b });
        const float minimum = std::min({ colour.r, colour.g, colour.b });
        const float delta = maximum - minimum;

        value = maximum;
        saturation = maximum <= 0.00001f ? 0.0f : delta / maximum;

        if (delta <= 0.00001f)
        {
            hue = 0.0f;
            return;
        }

        if (maximum == colour.r)
            hue = std::fmod((colour.g - colour.b) / delta, 6.0f) / 6.0f;
        else if (maximum == colour.g)
            hue = (((colour.b - colour.r) / delta) + 2.0f) / 6.0f;
        else
            hue = (((colour.r - colour.g) / delta) + 4.0f) / 6.0f;

        if (hue < 0.0f)
            hue += 1.0f;
    }

    // ------------------------------------------------------------------------
    // D2D helpers
    // ------------------------------------------------------------------------

    void SetBrush(const D2D1_COLOR_F& colour)
    {
        app.brush->SetColor(colour);
    }

    void FillRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& colour)
    {
        SetBrush(colour);
        app.d2dContext->FillRectangle(r, app.brush.Get());
    }

    void DrawRect(const D2D1_RECT_F& r, const D2D1_COLOR_F& colour, float stroke = 1.0f)
    {
        SetBrush(colour);
        app.d2dContext->DrawRectangle(r, app.brush.Get(), stroke);
    }

    void FillRound(const D2D1_RECT_F& r, float radius, const D2D1_COLOR_F& colour)
    {
        SetBrush(colour);
        app.d2dContext->FillRoundedRectangle(D2D1::RoundedRect(r, radius, radius), app.brush.Get());
    }

    void DrawRound(const D2D1_RECT_F& r, float radius, const D2D1_COLOR_F& colour, float stroke = 1.0f)
    {
        SetBrush(colour);
        app.d2dContext->DrawRoundedRectangle(D2D1::RoundedRect(r, radius, radius), app.brush.Get(), stroke);
    }

    void DrawLine(float x1, float y1, float x2, float y2,
        const D2D1_COLOR_F& colour, float stroke = 1.0f)
    {
        SetBrush(colour);
        app.d2dContext->DrawLine(
            D2D1::Point2F(x1, y1),
            D2D1::Point2F(x2, y2),
            app.brush.Get(),
            stroke);
    }

    void DrawTextValue(
        const std::wstring& value,
        const D2D1_RECT_F& r,
        IDWriteTextFormat* format,
        const D2D1_COLOR_F& colour,
        DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING)
    {
        format->SetTextAlignment(alignment);
        format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        SetBrush(colour);

        app.d2dContext->DrawTextW(
            value.c_str(),
            static_cast<UINT32>(value.size()),
            format,
            r,
            app.brush.Get(),
            D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    void DrawPanel(const D2D1_RECT_F& r, const std::wstring& title)
    {
        // Same transparent-card style as the supplied UI.
        DrawTextValue(
            title,
            Rect(r.left + 4, r.top + 8, r.right - 4, r.top + 34),
            app.text16Bold.Get(),
            ui.muted);
    }

    void DrawCheckbox(float x, float y, bool value)
    {
        const D2D1_RECT_F box = Rect(x, y, x + 18, y + 18);
        FillRound(box, 2.0f, value ? WithAlpha(app.settings.accentColour, 0.55f) : ui.background);
        DrawRound(box, 2.0f, value ? app.settings.accentColour : ui.border);

        if (value)
        {
            DrawLine(x + 4, y + 9, x + 8, y + 13, ui.white, 1.7f);
            DrawLine(x + 8, y + 13, x + 15, y + 5, ui.white, 1.7f);
        }
    }

    bool ToggleRow(const std::wstring& label, bool& value, float x, float y, float width)
    {
        const float rowH = 28.0f;
        const D2D1_RECT_F row = Rect(x, y, x + width, y + rowH);
        RegisterInteractive(row);
        const bool hovered = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), row);

        DrawTextValue(label, Rect(x, y, x + width - 48.0f, y + rowH),
            app.text13.Get(), hovered ? ui.text : MakeColour(0xD6D6D6));

        const float trackW = 31.0f;
        const float trackH = 14.0f;
        const float trackX = x + width - trackW;
        const float trackY = y + (rowH - trackH) * 0.5f;
        const D2D1_COLOR_F accentBright = D2D1::ColorF(
            Clamp(app.settings.accentColour.r * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.g * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.b * 1.22f + 0.03f, 0.0f, 1.0f), 1.0f);

        const D2D1_COLOR_F trackColour = value
            ? WithAlpha(app.settings.accentColour, hovered ? 0.58f : 0.44f)
            : (hovered ? MakeColour(0x232832) : MakeColour(0x191D24));

        FillRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f, trackColour);
        DrawRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f,
            value ? WithAlpha(accentBright, 0.42f) : MakeColour(0x252A33));

        const float knobSize = 12.0f;
        const float knobX = value ? trackX + trackW - knobSize - 1.0f : trackX + 1.0f;
        const float knobY = trackY + 1.0f;
        // Keep the toggle circle white in both states for a cleaner consistent look.
        FillRound(Rect(knobX, knobY, knobX + knobSize, knobY + knobSize), 6.0f,
            ui.white);

        DrawRound(Rect(knobX, knobY, knobX + knobSize, knobY + knobSize), 6.0f,
            value ? WithAlpha(app.settings.accentColour, 0.30f)
            : MakeColour(0x4A505A),
            1.0f);

        if (hovered && app.clicked && !app.clickConsumed && !app.colourPickerOpen && !app.healthStyleDropdownOpen)
        {
            value = !value;
            app.clickConsumed = true;
            return true;
        }
        return false;
    }


    void OpenColourPicker(D2D1_COLOR_F& colour);

    bool ToggleColourRow(
        const std::wstring& label,
        bool& value,
        D2D1_COLOR_F& colour,
        float x,
        float y,
        float width,
        VisualPopup popup = VisualPopup::None)
    {
        const float rowH = 28.0f;
        const D2D1_RECT_F row = Rect(x, y, x + width, y + rowH);
        const bool hovered = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), row);

        const float trackW = 31.0f;
        const float trackH = 14.0f;
        const float trackX = x + width - trackW;
        const float trackY = y + (rowH - trackH) * 0.5f;

        // Gear sits immediately to the left of the paint/drop icon.
        const D2D1_RECT_F paintRect =
            Rect(trackX - 31.0f, y + 1.0f, trackX - 5.0f, y + rowH - 1.0f);
        const D2D1_RECT_F gearRect =
            Rect(paintRect.left - 27.0f, y + 1.0f, paintRect.left - 2.0f, y + rowH - 1.0f);

        RegisterInteractive(row);
        RegisterInteractive(paintRect);
        if (popup != VisualPopup::None)
            RegisterInteractive(gearRect);

        const bool paintHovered = Hit(
            static_cast<float>(app.mouse.x),
            static_cast<float>(app.mouse.y),
            paintRect);
        const bool gearHovered =
            popup != VisualPopup::None &&
            Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), gearRect);
        const bool popupSelected = popup != VisualPopup::None && app.visualPopup == popup;

        DrawTextValue(
            label,
            Rect(x, y, gearRect.left - 5.0f, y + rowH),
            app.text13.Get(),
            hovered ? ui.text : MakeColour(0xD6D6D6));

        if (gearHovered || popupSelected)
        {
            FillRound(
                Rect(gearRect.left + 1.0f, gearRect.top + 2.0f,
                    gearRect.right - 1.0f, gearRect.bottom - 2.0f),
                6.0f,
                popupSelected
                ? WithAlpha(app.settings.accentColour, 0.18f)
                : WithAlpha(ui.hover, 0.55f));
        }

        if (popup != VisualPopup::None)
        {
            DrawTextValue(
                L"\xE713",
                gearRect,
                app.iconFont.Get(),
                popupSelected ? app.settings.accentColour : gearHovered ? ui.white : ui.muted,
                DWRITE_TEXT_ALIGNMENT_CENTER);
        }

        if (paintHovered)
        {
            FillRound(
                Rect(paintRect.left + 1.0f, paintRect.top + 2.0f,
                    paintRect.right - 1.0f, paintRect.bottom - 2.0f),
                6.0f,
                WithAlpha(colour, 0.13f));
        }

        DrawTextValue(
            L"\xE790",
            paintRect,
            app.iconFont.Get(),
            paintHovered ? ui.white : colour,
            DWRITE_TEXT_ALIGNMENT_CENTER);

        const D2D1_COLOR_F accentBright = D2D1::ColorF(
            Clamp(app.settings.accentColour.r * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.g * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.b * 1.22f + 0.03f, 0.0f, 1.0f),
            1.0f);

        const D2D1_COLOR_F trackColour = value
            ? WithAlpha(app.settings.accentColour, hovered ? 0.58f : 0.44f)
            : (hovered ? MakeColour(0x232832) : MakeColour(0x191D24));

        FillRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f, trackColour);
        DrawRound(
            Rect(trackX, trackY, trackX + trackW, trackY + trackH),
            7.0f,
            value ? WithAlpha(accentBright, 0.42f) : MakeColour(0x252A33));

        const float knobSize = 12.0f;
        const float knobX = value ? trackX + trackW - knobSize - 1.0f : trackX + 1.0f;
        const float knobY = trackY + 1.0f;
        // Keep the toggle circle white in both states for a cleaner consistent look.
        FillRound(
            Rect(knobX, knobY, knobX + knobSize, knobY + knobSize),
            6.0f,
            ui.white);

        DrawRound(
            Rect(knobX, knobY, knobX + knobSize, knobY + knobSize),
            6.0f,
            value ? WithAlpha(app.settings.accentColour, 0.30f)
            : MakeColour(0x4A505A),
            1.0f);

        if (!app.colourPickerOpen && !app.healthStyleDropdownOpen &&
            app.clicked && !app.clickConsumed)
        {
            if (gearHovered)
            {
                app.visualPopup = popupSelected ? VisualPopup::None : popup;
                app.visualPopupAnchorY = y;
                app.visualPopupPositioned = popupSelected;
                app.draggingVisualPopup = false;
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
                app.activeSlider = SliderId::None;
                app.clickConsumed = true;
                return false;
            }

            if (paintHovered)
            {
                app.visualPopup = VisualPopup::None;
                OpenColourPicker(colour);
                app.clickConsumed = true;
                return false;
            }

            if (hovered)
            {
                value = !value;
                app.clickConsumed = true;
                return true;
            }
        }

        return false;
    }

    void SliderControl(
        const std::wstring& label,
        SliderId id,
        float& value,
        float minimum,
        float maximum,
        float x,
        float y,
        float width,
        int decimals = 1)
    {
        DrawTextValue(label, Rect(x, y, x + 126.0f, y + 24.0f),
            app.text13.Get(), ui.text);

        wchar_t valueText[64]{};
        if (decimals == 0)
            swprintf_s(valueText, L"%.0f", value);
        else
            swprintf_s(valueText, L"%.*f", decimals, value);

        DrawTextValue(valueText, Rect(x + 126.0f, y, x + 170.0f, y + 24.0f),
            app.text13.Get(), ui.text, DWRITE_TEXT_ALIGNMENT_TRAILING);

        const float trackX = x + 181.0f;
        const float trackY = y + 14.0f;
        const float trackW = std::max(30.0f, width - 191.0f);
        const float ratio = Clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
        const float knobX = trackX + ratio * trackW;

        DrawLine(trackX, trackY, trackX + trackW, trackY, MakeColour(0x17191D, 0.8f), 3.0f);
        DrawLine(trackX, trackY, knobX, trackY, app.settings.accentColour, 3.0f);
        FillRound(Rect(knobX - 5.0f, trackY - 5.0f, knobX + 5.0f, trackY + 5.0f),
            5.0f, app.activeSlider == id ? ui.white : MakeColour(0xE7E7E7));

        Slider item;
        item.id = id;
        item.hit = Rect(trackX - 5.0f, trackY - 9.0f, trackX + trackW + 5.0f, trackY + 9.0f);
        item.value = &value;
        item.minimum = minimum;
        item.maximum = maximum;
        app.sliders.push_back(item);
        RegisterInteractive(item.hit);

        const bool hovered = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), item.hit);
        if (hovered && app.clicked && !app.clickConsumed && !app.colourPickerOpen && !app.healthStyleDropdownOpen)
        {
            app.activeSlider = id;
            app.clickConsumed = true;
        }

        if (app.mouseDown && app.activeSlider == id && !app.colourPickerOpen && !app.healthStyleDropdownOpen)
        {
            const float t = Clamp((static_cast<float>(app.mouse.x) - trackX) / trackW, 0.0f, 1.0f);
            value = minimum + t * (maximum - minimum);
            if (decimals == 0)
                value = std::round(value);
        }
    }

    void OpenColourPicker(D2D1_COLOR_F& colour)
    {
        app.colourTarget = &colour;
        app.originalColour = colour;
        app.workingColour = colour;
        ColourToHsv(colour, app.pickerHue, app.pickerSaturation, app.pickerValue);
        app.colourPickerOpen = true;
        app.healthStyleDropdownOpen = false;
        app.popupDropdown = PopupDropdown::None;
        app.activePickerArea = -1;
        app.activeSlider = SliderId::None;
    }

    void ColourRow(const std::wstring& label, D2D1_COLOR_F& colour,
        float x, float y, float width)
    {
        const float rowH = 29.0f;
        const float iconLeft = x + width - 31.0f;
        const float iconRight = x + width - 2.0f;
        const D2D1_RECT_F row = Rect(x, y, x + width, y + rowH);
        const D2D1_RECT_F iconRect = Rect(iconLeft, y, iconRight, y + rowH);
        RegisterInteractive(iconRect);
        const bool rowHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), row);
        const bool iconHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), iconRect);

        if (rowHover)
            FillRound(Rect(x - 5.0f, y + 1.0f, x + width + 1.0f, y + rowH - 1.0f),
                5.0f, WithAlpha(ui.hover, 0.32f));

        DrawTextValue(label, Rect(x, y, x + width - 36.0f, y + rowH),
            app.text13.Get(), rowHover ? ui.text : MakeColour(0xD5D5D5));

        if (iconHover)
            FillRound(Rect(iconLeft + 2.0f, y + 3.0f, iconRight - 1.0f, y + rowH - 3.0f),
                6.0f, WithAlpha(colour, 0.12f));

        app.iconFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        DrawTextValue(L"\xE790", iconRect, app.iconFont.Get(), colour, DWRITE_TEXT_ALIGNMENT_CENTER);

        if (!app.colourPickerOpen && !app.healthStyleDropdownOpen && iconHover && app.clicked && !app.clickConsumed)
        {
            OpenColourPicker(colour);
            app.clickConsumed = true;
        }
    }


    int PopupDropdownCount(PopupDropdown kind)
    {
        switch (kind)
        {
        case PopupDropdown::BoxStyle: return 5;
        case PopupDropdown::HealthStyle: return 6;
        case PopupDropdown::SnaplineOrigin: return 3;
        case PopupDropdown::SoundWalkAnimation: return 4;
        default: return 0;
        }
    }

    const wchar_t* PopupDropdownItem(PopupDropdown kind, int index)
    {
        static constexpr std::array<const wchar_t*, 5> boxStyles
        {
            L"Full",
            L"Corners",
            L"Full + corners",
            L"Filled",
            L"Filled + corners"
        };
        static constexpr std::array<const wchar_t*, 6> healthStyles
        {
            L"Classic",
            L"Thin",
            L"Segmented",
            L"Blocks",
            L"Split",
            L"Bottom"
        };
        static constexpr std::array<const wchar_t*, 3> lineOrigins
        {
            L"Bottom",
            L"Centre",
            L"Top"
        };
        static constexpr std::array<const wchar_t*, 4> soundWalkAnimations
        {
            L"Smooth",
            L"Cinematic",
            L"Fast",
            L"Minimal"
        };

        switch (kind)
        {
        case PopupDropdown::BoxStyle:
            return boxStyles[static_cast<std::size_t>(std::clamp(index, 0, 4))];
        case PopupDropdown::HealthStyle:
            return healthStyles[static_cast<std::size_t>(std::clamp(index, 0, 5))];
        case PopupDropdown::SnaplineOrigin:
            return lineOrigins[static_cast<std::size_t>(std::clamp(index, 0, 2))];
        case PopupDropdown::SoundWalkAnimation:
            return soundWalkAnimations[static_cast<std::size_t>(std::clamp(index, 0, 3))];
        default:
            return L"";
        }
    }

    int& PopupDropdownValue(PopupDropdown kind)
    {
        switch (kind)
        {
        case PopupDropdown::BoxStyle: return app.settings.boxStyle;
        case PopupDropdown::HealthStyle: return app.settings.healthBarStyle;
        case PopupDropdown::SnaplineOrigin: return app.settings.snaplineOrigin;
        default: return app.settings.soundWalkAnimationStyle;
        }
    }

    void PopupDropdownControl(
        const std::wstring& label,
        PopupDropdown kind,
        float x,
        float y,
        float width)
    {
        int& selected = PopupDropdownValue(kind);
        selected = std::clamp(selected, 0, PopupDropdownCount(kind) - 1);

        const float rowH = 30.0f;
        const float labelW = 72.0f;
        const float boxX = x + labelW;
        const float boxW = std::max(105.0f, width - labelW);
        const D2D1_RECT_F button = Rect(boxX, y, boxX + boxW, y + rowH);
        RegisterInteractive(button);
        const bool hovered = Hit(
            static_cast<float>(app.mouse.x),
            static_cast<float>(app.mouse.y),
            button);
        const bool open =
            app.healthStyleDropdownOpen && app.popupDropdown == kind;

        app.healthStyleButtonRect = button;
        if (open)
        {
            app.healthStylePopupRect = Rect(
                button.left,
                button.bottom + 4.0f,
                button.right,
                button.bottom + 4.0f +
                27.0f * static_cast<float>(PopupDropdownCount(kind)));
        }

        DrawTextValue(
            label,
            Rect(x, y, boxX - 7.0f, y + rowH),
            app.text12.Get(),
            MakeColour(0xD6D6D6));

        FillRound(
            button,
            1.0f,
            hovered || open ? MakeColour(0x0B0D11) : MakeColour(0x050609));
        DrawRound(
            button,
            1.0f,
            open ? WithAlpha(app.settings.accentColour, 0.78f)
            : MakeColour(0x171A20),
            1.0f);

        DrawTextValue(
            PopupDropdownItem(kind, selected),
            Rect(button.left + 9.0f, button.top, button.right - 25.0f, button.bottom),
            app.text12.Get(),
            ui.text);

        DrawTextValue(
            L"\xE70D",
            Rect(button.right - 23.0f, button.top, button.right - 4.0f, button.bottom),
            app.iconFont.Get(),
            open ? app.settings.accentColour : ui.muted,
            DWRITE_TEXT_ALIGNMENT_CENTER);

        if (!app.colourPickerOpen && hovered && app.clicked && !app.clickConsumed)
        {
            if (open)
            {
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
            }
            else
            {
                app.healthStyleDropdownOpen = true;
                app.popupDropdown = kind;
                app.healthStylePopupRect = Rect(
                    button.left,
                    button.bottom + 4.0f,
                    button.right,
                    button.bottom + 4.0f +
                    27.0f * static_cast<float>(PopupDropdownCount(kind)));
            }

            app.activeSlider = SliderId::None;
            app.clickConsumed = true;
        }
    }

    void DrawHealthStyleDropdownOverlay()
    {
        if (!app.healthStyleDropdownOpen ||
            app.popupDropdown == PopupDropdown::None ||
            app.colourPickerOpen)
        {
            return;
        }

        const PopupDropdown kind = app.popupDropdown;
        const int count = PopupDropdownCount(kind);
        int& selectedValue = PopupDropdownValue(kind);
        const D2D1_RECT_F popup = app.healthStylePopupRect;

        FillRound(
            Rect(popup.left + 4.0f, popup.top + 5.0f,
                popup.right + 4.0f, popup.bottom + 5.0f),
            1.0f,
            MakeColour(0x000000, 0.48f));
        FillRound(popup, 1.0f, MakeColour(0x030406, 0.99f));
        DrawRound(popup, 1.0f, WithAlpha(app.settings.accentColour, 0.72f));

        constexpr float itemH = 27.0f;
        bool handled = false;

        for (int index = 0; index < count; ++index)
        {
            const float top = popup.top + itemH * static_cast<float>(index);
            const D2D1_RECT_F item = Rect(popup.left, top, popup.right, top + itemH);
            RegisterInteractive(item);
            const bool hovered = Hit(
                static_cast<float>(app.mouse.x),
                static_cast<float>(app.mouse.y),
                item);
            const bool selected = index == selectedValue;

            if (hovered)
            {
                FillRound(
                    Rect(item.left + 3.0f, item.top + 2.0f,
                        item.right - 3.0f, item.bottom - 2.0f),
                    1.0f,
                    MakeColour(0x0B0D11));
            }

            if (selected)
            {
                FillRound(
                    Rect(item.left + 5.0f, item.top + 7.0f,
                        item.left + 8.0f, item.bottom - 7.0f),
                    1.0f,
                    app.settings.accentColour);
            }

            DrawTextValue(
                PopupDropdownItem(kind, index),
                Rect(item.left + 13.0f, item.top, item.right - 10.0f, item.bottom),
                app.text12.Get(),
                selected ? app.settings.accentColour : hovered ? ui.text : ui.muted);

            if (hovered && app.clicked && !app.clickConsumed)
            {
                selectedValue = index;
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
                app.clickConsumed = true;
                handled = true;
                break;
            }
        }

        if (!handled && app.clicked && !app.clickConsumed)
        {
            const float mx = static_cast<float>(app.mouse.x);
            const float my = static_cast<float>(app.mouse.y);
            const bool onButton = Hit(mx, my, app.healthStyleButtonRect);
            const bool onPopup = Hit(mx, my, popup);

            if (!onButton && !onPopup)
            {
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
                app.clickConsumed = true;
            }
        }
    }

