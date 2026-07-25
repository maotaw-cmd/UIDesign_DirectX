#pragma once

#include "Common.h"


#ifndef NOMINMAX
#define NOMINMAX
#endif

// ============================================================================
// CustomizationUI.h
// Edit this file to change the window colours and default visual UI settings.
// It intentionally contains data/defaults only, so it is safe to edit without
// touching rendering, config persistence, or the Win32 message loop.
// ============================================================================


using namespace DirectX;

struct Colours
{
    D2D1_COLOR_F background = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.90f);
    D2D1_COLOR_F top = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.76f);
    D2D1_COLOR_F panel = D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.66f);
    D2D1_COLOR_F row = D2D1::ColorF(0.015f, 0.015f, 0.020f, 0.72f);
    D2D1_COLOR_F hover = D2D1::ColorF(0x24 / 255.0f, 0x12 / 255.0f, 0x42 / 255.0f, 0.70f);
    D2D1_COLOR_F border = D2D1::ColorF(0x2A / 255.0f, 0x2A / 255.0f, 0x38 / 255.0f, 0.72f);
    D2D1_COLOR_F text = D2D1::ColorF(0xEE / 255.0f, 0xEE / 255.0f, 0xF5 / 255.0f, 1.0f);
    D2D1_COLOR_F muted = D2D1::ColorF(0x94 / 255.0f, 0x95 / 255.0f, 0xAE / 255.0f, 1.0f);
    D2D1_COLOR_F track = D2D1::ColorF(0x20 / 255.0f, 0x20 / 255.0f, 0x38 / 255.0f, 1.0f);
    D2D1_COLOR_F white = D2D1::ColorF(0xF4 / 255.0f, 0xF5 / 255.0f, 0xF7 / 255.0f, 1.0f);
};

inline Colours ui{};

inline D2D1_COLOR_F MakeColour(std::uint32_t rgb, float alpha = 1.0f)
{
    return D2D1::ColorF(
        static_cast<float>((rgb >> 16) & 0xFF) / 255.0f,
        static_cast<float>((rgb >> 8) & 0xFF) / 255.0f,
        static_cast<float>(rgb & 0xFF) / 255.0f,
        alpha);
}

inline D2D1_COLOR_F WithAlpha(const D2D1_COLOR_F& c, float alpha)
{
    return D2D1::ColorF(c.r, c.g, c.b, alpha);
}

inline float Clamp(float v, float minimum, float maximum)
{
    return (std::max)(minimum, (std::min)(v, maximum));
}

inline bool Hit(float x, float y, const D2D1_RECT_F& r)
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
    bool weapon = true;
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
    D2D1_COLOR_F healthTextColour = MakeColour(0xFFFFFF);
    D2D1_COLOR_F nameColour = MakeColour(0xFFFFFF);
    D2D1_COLOR_F distanceColour = MakeColour(0x9495AE);
    D2D1_COLOR_F weaponColour = MakeColour(0xFFFFFF);
    D2D1_COLOR_F skeletonColour = MakeColour(0x65B8FF);
    D2D1_COLOR_F snaplineColour = MakeColour(0xFF8E44);
    D2D1_COLOR_F soundWalkColour = MakeColour(0xB8E7FF);
    D2D1_COLOR_F soundMarkerColour = MakeColour(0xF6C160);
    D2D1_COLOR_F visualGlowColour = MakeColour(0xFFFFFF);
    D2D1_COLOR_F modelColour = MakeColour(0xF8D6D6);
    D2D1_COLOR_F visibleCheckColour = MakeColour(0xFFFFFF);
    D2D1_COLOR_F teamCheckColour = MakeColour(0xF8D6D6);
    D2D1_COLOR_F accentColour = MakeColour(0xFD5A5A);
};
