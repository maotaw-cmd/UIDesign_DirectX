#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace PreviewRenderer
{
    struct Settings
    {
        float characterScale = 1.00f;
        bool particles = true;
        float particleSpeed = 22.0f;
        float particleAmount = 34.0f;

        bool box = true;
        bool cornerBox = false;
        bool filled = true;
        bool healthBar = true;
        bool name = true;
        bool distance = false;
        bool snapline = false;
        bool skeleton = false;
        bool weapon = false;
        bool damage = true;
        bool visualGlow = true;
        float visualGlowThickness = 2.0f;
        int healthStyle = 1;
        float healthBarWidth = 6.0f;

        float accent[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float boxColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float filledColour[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float healthColour[4] = { 0.258f, 0.839f, 0.420f, 1.0f };
        float healthBackColour[4] = { 0.02f, 0.025f, 0.03f, 1.0f };
        float damageColour[4] = { 1.0f, 0.25f, 0.31f, 1.0f };
        float visualGlowColour[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float lineColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float skeletonColour[4] = { 0.835f, 0.859f, 1.0f, 1.0f };
        float backgroundColour[4] = { 0.004f, 0.007f, 0.011f, 1.0f };
    };

    bool Initialize(HWND window);
    void SetSettings(const Settings& settings);
    void Render();
    void Shutdown();
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
}
