#pragma once

// ============================================================================
// BEGINNER CUSTOMIZATION
// Change common colours, sizes and defaults here.
// ============================================================================

namespace BeginnerCustomization
{
    inline constexpr int MainWindowWidth = 568;
    inline constexpr int MainWindowHeight = 370;
    inline constexpr int PreviewWindowWidth = 270;
    inline constexpr int PreviewWindowGap = 10;

    inline constexpr int WindowCornerRadius = 1;
    inline constexpr float SidebarWidth = 54.0f;
    inline constexpr float ContentShift = 66.0f;

    // 0.90 = 90% opacity.
    inline constexpr float DefaultWindowOpacity = 0.90f;

    // 0xRRGGBB
    inline constexpr unsigned MainBackground = 0x111416;
    inline constexpr unsigned SidebarBackground = 0x101315;
    inline constexpr unsigned WindowOutline = 0x252B2E;
    inline constexpr unsigned DividerColour = 0x252A2D;

    inline constexpr float EnemyR = 0.94f;
    inline constexpr float EnemyG = 0.30f;
    inline constexpr float EnemyB = 0.33f;

    inline constexpr float LocalR = 0.23f;
    inline constexpr float LocalG = 0.70f;
    inline constexpr float LocalB = 0.96f;

    inline constexpr float CharacterScale = 0.84f;
    inline constexpr float DefaultGlowThicknessSlider = 0.10f;
    inline constexpr float DefaultBarWidthSlider = 1.0f / 11.0f;
}
