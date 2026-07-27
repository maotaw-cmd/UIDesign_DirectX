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
        float characterScale = 0.84f;
        bool particles = true;
        float particleSpeed = 22.0f;
        float particleAmount = 34.0f;

        bool box = true;
        bool cornerBox = true;
        bool filled = false;
        bool healthBar = true;
        bool armorBar = false;
        bool name = true;
        bool distance = false;
        bool snapline = false;
        bool skeleton = false;
        bool weapon = false;
        bool damage = true;
        bool visualGlow = true;
        bool characterChams = true;
        int visualGlowMode = 0; // 0 soft, 1 pulse, 2 dynamic, 3 rainbow, 4 neon
        int characterChamsStyle = 0; // 0 solid, 1 pulse, 2 rainbow, 3 metallic
        float visualGlowThickness = 2.0f;
        int healthStyle = 1;
        int armorStyle = 1;
        int healthPosition = 0;
        int armorPosition = 0;
        float healthBarWidth = 2.0f;
        float armorBarWidth = 2.0f;

        float accent[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float boxColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float filledColour[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float healthColour[4] = { 0.258f, 0.839f, 0.420f, 1.0f };
        float healthBackColour[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float armorColour[4] = { 0.20f, 0.55f, 1.0f, 1.0f };
        float armorBackColour[4] = { 0.035f, 0.10f, 0.22f, 1.0f };
        float damageColour[4] = { 1.0f, 0.25f, 0.31f, 1.0f };
        float visualGlowColour[4] = { 0.70f, 0.086f, 0.133f, 1.0f };
        float characterColour[4] = { 0.63f, 0.66f, 0.72f, 1.0f };
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
