


// Sound-walk popup controls and their editable UI logic.
// Included once through ProjectAll.h. Do not add this file to Source Files.

// ===== BEGIN UI/SoundWalkSettings.hpp =====

// ============================================================================
// SoundWalkSettings.hpp
// Everything related to the Sound Walk Visuals row and its popup lives here.
// The main Settings page intentionally contains no Sound Walk animation controls.
// ============================================================================

namespace SoundWalkUI
{
    constexpr float PopupHeight = 238.0f;

    inline void DrawRow(float x, float y, float width)
    {
        const float rowHeight = 28.0f;
        const D2D1_RECT_F row = Rect(x, y, x + width, y + rowHeight);

        const float toggleWidth = 31.0f;
        const float toggleHeight = 14.0f;
        const float toggleX = x + width - toggleWidth;
        const float toggleY = y + (rowHeight - toggleHeight) * 0.5f;

        // Required order: [gear] [paint] [toggle].
        const D2D1_RECT_F paintRect =
            Rect(toggleX - 31.0f, y + 1.0f, toggleX - 5.0f, y + rowHeight - 1.0f);
        const D2D1_RECT_F gearRect =
            Rect(paintRect.left - 27.0f, y + 1.0f, paintRect.left - 2.0f, y + rowHeight - 1.0f);

        RegisterInteractive(row);
        RegisterInteractive(gearRect);
        RegisterInteractive(paintRect);

        const float mouseX = static_cast<float>(app.mouse.x);
        const float mouseY = static_cast<float>(app.mouse.y);
        const bool rowHovered = Hit(mouseX, mouseY, row);
        const bool gearHovered = Hit(mouseX, mouseY, gearRect);
        const bool paintHovered = Hit(mouseX, mouseY, paintRect);
        const bool popupOpen = app.visualPopup == VisualPopup::SoundWalk;

        DrawTextValue(
            L"Sound walk",
            Rect(x, y, gearRect.left - 5.0f, y + rowHeight),
            app.text13.Get(),
            rowHovered ? ui.text : MakeColour(0xD6D6D6));

        // Keep the gear clearly visible even when it is not hovered.
        FillRound(
            Rect(gearRect.left + 1.0f, gearRect.top + 2.0f,
                 gearRect.right - 1.0f, gearRect.bottom - 2.0f),
            1.0f,
            popupOpen
                ? WithAlpha(app.settings.accentColour, 0.20f)
                : gearHovered
                    ? MakeColour(0x111418, 0.72f)
                    : MakeColour(0x090B0E, 0.68f));
        DrawRound(
            Rect(gearRect.left + 1.0f, gearRect.top + 2.0f,
                 gearRect.right - 1.0f, gearRect.bottom - 2.0f),
            1.0f,
            popupOpen ? app.settings.accentColour : MakeColour(0x252A33),
            1.0f);
        DrawTextValue(
            L"\xE713",
            gearRect,
            app.iconFont.Get(),
            popupOpen ? app.settings.accentColour : gearHovered ? ui.white : MakeColour(0xB8BDC7),
            DWRITE_TEXT_ALIGNMENT_CENTER);

        if (paintHovered)
        {
            FillRound(
                Rect(paintRect.left + 1.0f, paintRect.top + 2.0f,
                     paintRect.right - 1.0f, paintRect.bottom - 2.0f),
                1.0f,
                WithAlpha(app.settings.soundWalkColour, 0.13f));
        }

        DrawTextValue(
            L"\xE790",
            paintRect,
            app.iconFont.Get(),
            paintHovered ? ui.white : app.settings.soundWalkColour,
            DWRITE_TEXT_ALIGNMENT_CENTER);

        const D2D1_COLOR_F accentBright = D2D1::ColorF(
            Clamp(app.settings.accentColour.r * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.g * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.b * 1.22f + 0.03f, 0.0f, 1.0f),
            1.0f);

        const D2D1_COLOR_F trackColour = app.settings.soundWalk
            ? WithAlpha(app.settings.accentColour, rowHovered ? 0.58f : 0.44f)
            : (rowHovered ? MakeColour(0x232832) : MakeColour(0x191D24));

        FillRound(
            Rect(toggleX, toggleY, toggleX + toggleWidth, toggleY + toggleHeight),
            7.0f,
            trackColour);
        DrawRound(
            Rect(toggleX, toggleY, toggleX + toggleWidth, toggleY + toggleHeight),
            7.0f,
            app.settings.soundWalk
                ? WithAlpha(accentBright, 0.42f)
                : MakeColour(0x252A33),
            1.0f);

        const float knobSize = 12.0f;
        const float knobX = app.settings.soundWalk
            ? toggleX + toggleWidth - knobSize - 1.0f
            : toggleX + 1.0f;
        const float knobY = toggleY + 1.0f;

        FillRound(
            Rect(knobX, knobY, knobX + knobSize, knobY + knobSize),
            6.0f,
            ui.white);
        DrawRound(
            Rect(knobX, knobY, knobX + knobSize, knobY + knobSize),
            6.0f,
            app.settings.soundWalk
                ? WithAlpha(app.settings.accentColour, 0.30f)
                : MakeColour(0x4A505A),
            1.0f);

        if (app.colourPickerOpen || app.healthStyleDropdownOpen ||
            !app.clicked || app.clickConsumed)
        {
            return;
        }

        if (gearHovered)
        {
            app.visualPopup = popupOpen ? VisualPopup::None : VisualPopup::SoundWalk;
            app.visualPopupAnchorY = y;
            app.visualPopupPositioned = popupOpen;
            app.draggingVisualPopup = false;
            app.healthStyleDropdownOpen = false;
            app.popupDropdown = PopupDropdown::None;
            app.activeSlider = SliderId::None;
            app.clickConsumed = true;
            return;
        }

        if (paintHovered)
        {
            app.visualPopup = VisualPopup::None;
            OpenColourPicker(app.settings.soundWalkColour);
            app.clickConsumed = true;
            return;
        }

        if (rowHovered)
        {
            app.settings.soundWalk = !app.settings.soundWalk;
            app.clickConsumed = true;
        }
    }

    inline void DrawPopupContents(float x, float y, float width)
    {
        PopupDropdownControl(
            L"Animation style",
            PopupDropdown::SoundWalkAnimation,
            x,
            y,
            width);

        SliderControl(
            L"Speed",
            SliderId::SoundWalkSpeed,
            app.settings.soundWalkSpeed,
            0.35f,
            1.80f,
            x,
            y + 43.0f,
            width,
            2);

        SliderControl(
            L"Expansion",
            SliderId::SoundWalkExpansion,
            app.settings.soundWalkExpansion,
            0.70f,
            2.30f,
            x,
            y + 82.0f,
            width,
            2);

        SliderControl(
            L"Line width",
            SliderId::SoundWalkThickness,
            app.settings.soundWalkThickness,
            0.60f,
            4.00f,
            x,
            y + 121.0f,
            width,
            1);

        DrawTextValue(
            L"Only the Sound Walk rings use these animation settings.",
            Rect(x, y + 160.0f, x + width, y + 190.0f),
            app.text12.Get(),
            ui.muted);
    }
}

// ===== END UI/SoundWalkSettings.hpp =====
