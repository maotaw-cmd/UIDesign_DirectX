


// Reusable beginner-editable UI widgets: rows, sliders, colour picker and dropdowns.
// Included once through ProjectAll.h. Do not add this file to Source Files.

    void ColourToHsv(const D2D1_COLOR_F& colour, float& hue, float& saturation, float& value)
    {
        const float maximum = (std::max)({ colour.r, colour.g, colour.b });
        const float minimum = (std::min)({ colour.r, colour.g, colour.b });
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


    float AnimatedTogglePosition(const bool* key, bool value)
    {
        float& position = app.toggleAnimations[key];
        if (position == 0.0f && value)
            position = 1.0f;

        const float target = value ? 1.0f : 0.0f;
        const float speed = 0.18f;
        position += (target - position) * speed;
        if (std::fabs(target - position) < 0.002f)
            position = target;
        return position;
    }

    void DrawPanel(const D2D1_RECT_F& r, const std::wstring& title)
    {
        FillRound(r, 7.0f, ui.panel);
        DrawTextValue(
            title,
            Rect(r.left + 16, r.top + 8, r.right - 16, r.top + 36),
            app.text14.Get(),
            app.settings.accentColour);
        DrawLine(r.left + 16, r.top + 38, r.right - 16, r.top + 38, WithAlpha(ui.border, 0.72f));
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
            ? WithAlpha(app.settings.accentColour, hovered ? 0.95f : 0.82f)
            : (hovered ? MakeColour(0x34344D) : MakeColour(0x25253A));

        FillRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f, trackColour);
        DrawRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f,
            value ? WithAlpha(accentBright, 0.42f) : MakeColour(0x252A33));

        const float knobSize = 12.0f;
        const float toggleT = AnimatedTogglePosition(&value, value);
        const float knobX = trackX + 1.0f + toggleT * (trackW - knobSize - 2.0f);
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
            ? WithAlpha(app.settings.accentColour, hovered ? 0.95f : 0.82f)
            : (hovered ? MakeColour(0x34344D) : MakeColour(0x25253A));

        FillRound(Rect(trackX, trackY, trackX + trackW, trackY + trackH), 7.0f, trackColour);
        DrawRound(
            Rect(trackX, trackY, trackX + trackW, trackY + trackH),
            7.0f,
            value ? WithAlpha(accentBright, 0.42f) : MakeColour(0x252A33));

        const float knobSize = 12.0f;
        const float toggleT = AnimatedTogglePosition(&value, value);
        const float knobX = trackX + 1.0f + toggleT * (trackW - knobSize - 2.0f);
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
        const float trackW = (std::max)(30.0f, width - 191.0f);
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
        const float boxW = (std::max)(105.0f, width - labelW);
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


