


// Main Visuals and Settings page controls. Add or remove menu rows here.
// Included once through ProjectAll.h. Do not add this file to Source Files.

// ===== BEGIN UI/VisualsUI.hpp =====

    // ------------------------------------------------------------------------
    // UI rendering - supplied dark UI style, visuals left / 3D character right
    // ------------------------------------------------------------------------

    D2D1_COLOR_F AccentBright()
    {
        return D2D1::ColorF(
            Clamp(app.settings.accentColour.r * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.g * 1.22f + 0.03f, 0.0f, 1.0f),
            Clamp(app.settings.accentColour.b * 1.22f + 0.03f, 0.0f, 1.0f), 1.0f);
    }

    void DrawIcon(const wchar_t* icon, const D2D1_RECT_F& rect, const D2D1_COLOR_F& colour)
    {
        DrawTextValue(icon, rect, app.iconFont.Get(), colour, DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    void DrawTriangleLogo(float x, float y)
    {
        const D2D1_COLOR_F glow = WithAlpha(ui.white, 0.14f);
        for (float o = 5.0f; o >= 1.0f; o -= 1.0f)
        {
            DrawLine(x + 2.0f, y + 24.0f, x + 15.0f, y + 2.0f, glow, o);
            DrawLine(x + 15.0f, y + 2.0f, x + 28.0f, y + 24.0f, glow, o);
            DrawLine(x + 28.0f, y + 24.0f, x + 2.0f, y + 24.0f, glow, o);
        }
        DrawLine(x + 2.0f, y + 24.0f, x + 15.0f, y + 2.0f, ui.white, 2.4f);
        DrawLine(x + 15.0f, y + 2.0f, x + 28.0f, y + 24.0f, ui.white, 2.4f);
        DrawLine(x + 28.0f, y + 24.0f, x + 2.0f, y + 24.0f, ui.white, 2.4f);
        DrawLine(x + 9.0f, y + 20.0f, x + 15.0f, y + 10.0f, WithAlpha(ui.white, 0.75f), 1.3f);
        DrawLine(x + 15.0f, y + 10.0f, x + 21.0f, y + 20.0f, WithAlpha(ui.white, 0.75f), 1.3f);
        DrawLine(x + 21.0f, y + 20.0f, x + 9.0f, y + 20.0f, WithAlpha(ui.white, 0.75f), 1.3f);
    }

    D2D1_RECT_F CloseButtonRect()
    {
        return Rect(static_cast<float>(app.width) - 52.0f, 10.0f, static_cast<float>(app.width) - 12.0f, 50.0f);
    }

    D2D1_RECT_F GearButtonRect()
    {
        return Rect(static_cast<float>(app.width) - 100.0f, 10.0f, static_cast<float>(app.width) - 60.0f, 50.0f);
    }

    D2D1_RECT_F SaveButtonRect()
    {
        return Rect(static_cast<float>(app.width) - 148.0f, 10.0f, static_cast<float>(app.width) - 108.0f, 50.0f);
    }

    void DrawTopBar()
    {
        FillRect(Rect(0, 0, static_cast<float>(app.width), TOP_H), ui.top);
        DrawTriangleLogo(22.0f, 17.0f);

        const D2D1_RECT_F saveRect = SaveButtonRect();
        const D2D1_RECT_F gearRect = GearButtonRect();
        const D2D1_RECT_F closeRect = CloseButtonRect();
        RegisterInteractive(saveRect); RegisterInteractive(gearRect); RegisterInteractive(closeRect);
        const float mx = static_cast<float>(app.mouse.x), my = static_cast<float>(app.mouse.y);
        const bool saveHover = Hit(mx,my,saveRect), gearHover = Hit(mx,my,gearRect), closeHover = Hit(mx,my,closeRect);
        if (saveHover) FillRound(saveRect, 7.0f, WithAlpha(ui.hover, 0.65f));
        if (gearHover) FillRound(gearRect, 7.0f, WithAlpha(ui.hover, 0.65f));
        if (closeHover) FillRound(closeRect, 7.0f, MakeColour(0x4A1731, 0.72f));
        DrawIcon(L"\xE74E", Rect(saveRect.left+8,saveRect.top+8,saveRect.right-8,saveRect.bottom-8), ui.white);
        DrawIcon(L"\xE713", Rect(gearRect.left+8,gearRect.top+8,gearRect.right-8,gearRect.bottom-8), ui.white);
        DrawIcon(L"\xE711", Rect(closeRect.left+8,closeRect.top+8,closeRect.right-8,closeRect.bottom-8), ui.white);
        DrawLine(14.0f, TOP_H-1.0f, static_cast<float>(app.width)-14.0f, TOP_H-1.0f, WithAlpha(ui.border,0.55f));
    }

    void DrawParticles(float width, float height)
    {
        if (!app.particlesEnabled)
            return;

        const float time = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
        const int count = (std::clamp)(static_cast<int>(std::round(app.particleAmount)), 8, 70);
        const float contentTop = TOP_H + 6.0f;
        const float contentHeight = (std::max)(1.0f, height - contentTop - 8.0f);
        const float contentWidth = (std::max)(1.0f, width - 18.0f);

        for (int index = 0; index < count; ++index)
        {
            const float seed = static_cast<float>(index + 1);
            const float baseX = std::fmod(seed * 91.73f + 27.0f, contentWidth);
            const float speed = app.particleSpeed * (0.42f + std::fmod(seed * 0.173f, 0.72f));
            const float travel = std::fmod(time * speed + seed * 31.0f, contentHeight + 24.0f);
            const float x = 10.0f + baseX + std::sin(time * 0.36f + seed * 1.7f) * 8.0f;
            const float y = contentTop + contentHeight + 12.0f - travel;
            const float radius = 0.75f + std::fmod(seed * 0.39f, 1.35f);
            const float alpha = 0.10f + std::fmod(seed * 0.061f, 0.16f);
            SetBrush(WithAlpha(app.settings.accentColour, alpha * 1.35f));
            app.d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), app.brush.Get());
        }
    }

    void DrawVisualControls(const D2D1_RECT_F& panel)
    {
        // Flat left panel: no outer outline, matching the requested cleaner layout.
        FillRound(panel, 7.0f, ui.panel);

        const float left = panel.left + 18.0f;
        const float right = panel.right - 18.0f;
        const float innerW = right - left;
        const float rowH = 29.0f;
        const float firstY = panel.top + 52.0f - app.visualScroll;

        const D2D1_RECT_F clipRect = Rect(panel.left + 2.0f, panel.top + 42.0f,
            panel.right - 9.0f, panel.bottom - 8.0f);
        app.visualScrollRect = clipRect;

        FillRound(Rect(left, panel.top + 20.0f, left + 4.0f, panel.top + 24.0f), 2.0f, ui.white);
        DrawTextValue(L"PLAYER ESP", Rect(left + 12.0f, panel.top + 9.0f, right, panel.top + 38.0f),
            app.text14.Get(), ui.white);
        FillRound(Rect(left + 98.0f, panel.top + 20.0f, left + 102.0f, panel.top + 24.0f), 2.0f, ui.white);
        DrawLine(left, panel.top + 40.0f, right, panel.top + 40.0f, WithAlpha(ui.border, 0.62f));

        app.d2dContext->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        auto toggleControl = [&](bool& value, float x, float y)
        {
            // Match the compact INFORMATION toggles exactly.
            const float trackW = 31.0f;
            const float trackH = 14.0f;
            const float trackY = y + 9.0f;
            const D2D1_RECT_F hit = Rect(x, trackY, x + trackW, trackY + trackH);
            RegisterInteractive(hit);
            const bool hover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), hit);

            FillRound(hit, 7.0f, value
                ? WithAlpha(app.settings.accentColour, hover ? 0.95f : 0.82f)
                : MakeColour(hover ? 0x34344D : 0x25253A));
            DrawRound(hit, 7.0f, value
                ? WithAlpha(AccentBright(), 0.42f)
                : MakeColour(0x252A33));

            const float knobSize = 12.0f;
            const float toggleT = AnimatedTogglePosition(&value, value);
            const float knobX = hit.left + 1.0f + toggleT * (trackW - knobSize - 2.0f);
            const float knobY = hit.top + 1.0f;
            FillRound(Rect(knobX, knobY, knobX + knobSize, knobY + knobSize), 6.0f, ui.white);
            DrawRound(Rect(knobX, knobY, knobX + knobSize, knobY + knobSize), 6.0f,
                value ? WithAlpha(app.settings.accentColour, 0.30f) : MakeColour(0x4A505A), 1.0f);

            if (hover && app.clicked && !app.clickConsumed && !app.colourPickerOpen)
            {
                value = !value;
                app.clickConsumed = true;
            }
        };

        auto gear = [&](VisualPopup popup, float x, float y)
        {
            const D2D1_RECT_F r = Rect(x, y + 3.0f, x + 30.0f, y + 31.0f);
            RegisterInteractive(r);
            const bool hover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), r);
            const bool active = app.visualPopup == popup;
            if (hover || active)
                FillRound(r, 4.0f, active ? WithAlpha(app.settings.accentColour, 0.18f) : ui.row);
            DrawTextValue(L"\xE713", r, app.iconFont.Get(), active ? app.settings.accentColour : hover ? ui.white : ui.muted,
                DWRITE_TEXT_ALIGNMENT_CENTER);
            if (hover && app.clicked && !app.clickConsumed)
            {
                app.visualPopup = active ? VisualPopup::None : popup;
                app.visualPopupPositioned = false;
                app.visualPopupAnchorY = y;
                app.clickConsumed = true;
            }
        };

        auto swatch = [&](D2D1_COLOR_F& colour, float x, float y, bool checker = false)
        {
            // Paint icon replaces the old rectangular colour swatch.
            const D2D1_RECT_F r = Rect(x + 1.0f, y + 3.0f, x + 30.0f, y + 31.0f);
            RegisterInteractive(r);
            const bool hover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), r);
            if (hover)
                FillRound(r, 4.0f, WithAlpha(ui.hover, 0.62f));

            const D2D1_COLOR_F iconColour = checker ? ui.muted : (hover ? ui.white : colour);
            DrawTextValue(L"\xE790", r, app.iconFont.Get(), iconColour,
                DWRITE_TEXT_ALIGNMENT_CENTER);

            if (hover && app.clicked && !app.clickConsumed)
            {
                OpenColourPicker(colour);
                app.clickConsumed = true;
            }
        };

        auto simpleRow = [&](const wchar_t* label, bool& value, float y,
            D2D1_COLOR_F* colour = nullptr, VisualPopup popup = VisualPopup::None,
            bool checker = false)
        {
            DrawTextValue(label, Rect(left + 6.0f, y, left + 155.0f, y + rowH),
                app.text13.Get(), ui.text);
            // Keep every row consistent: label on the left, then toggle, settings and colour on the right.
            const float colourX = right - 28.0f;
            const float gearX = right - 65.0f;
            const float toggleX = right - 105.0f;
            toggleControl(value, toggleX, y);
            if (popup != VisualPopup::None)
                gear(popup, gearX, y);
            if (colour)
                swatch(*colour, colourX, y, checker);
        };

        float y = firstY;
        simpleRow(L"Box", app.settings.box, y, &app.settings.boxColour, VisualPopup::Box); y += rowH;

        bool fillEnabled = app.settings.boxStyle == 3 || app.settings.boxStyle == 4;
        DrawTextValue(L"Fill", Rect(left + 6.0f, y, left + 155.0f, y + rowH), app.text13.Get(), ui.text);
        toggleControl(fillEnabled, right - 105.0f, y);
        if (fillEnabled && app.settings.boxStyle < 3) app.settings.boxStyle = 3;
        if (!fillEnabled && app.settings.boxStyle >= 3) app.settings.boxStyle = 1;
        swatch(app.settings.boxFillColour, right - 28.0f, y, !fillEnabled); y += rowH;

        simpleRow(L"Health bar", app.settings.healthBar, y, &app.settings.healthColour, VisualPopup::HealthBar); y += rowH;
        simpleRow(L"Health text", app.settings.healthText, y, &app.settings.visualGlowColour); y += rowH;
        simpleRow(L"Damage text", app.settings.damageText, y, &app.settings.damageTextColour); y += rowH;
        simpleRow(L"Skeleton", app.settings.skeleton, y, &app.settings.skeletonColour, VisualPopup::Skeleton); y += rowH;
        simpleRow(L"3D outline", app.settings.visualGlow, y, &app.settings.visualGlowColour, VisualPopup::ModelOutline); y += rowH;
        simpleRow(L"Name", app.settings.name, y, &app.settings.visualGlowColour); y += rowH;
        simpleRow(L"Distance", app.settings.distance, y, &app.settings.visualGlowColour); y += rowH;
        simpleRow(L"Snap line", app.settings.snapline, y, &app.settings.snaplineColour, VisualPopup::Snapline); y += rowH;
        simpleRow(L"Sound marker", app.settings.soundMarker, y, &app.settings.soundMarkerColour); y += rowH;
        simpleRow(L"Sound walk", app.settings.soundWalk, y, &app.settings.soundWalkColour, VisualPopup::SoundWalk); y += rowH + 4.0f;

        DrawLine(left, y, right, y, WithAlpha(ui.border, 0.65f));
        FillRound(Rect(left, y + 17.0f, left + 4.0f, y + 21.0f), 2.0f, ui.white);
        DrawTextValue(L"INFORMATION", Rect(left + 12.0f, y + 6.0f, right, y + 34.0f), app.text13.Get(), ui.white);
        y += 38.0f;

        // One control per row. Never place two toggles beside each other.
        simpleRow(L"Name", app.settings.name, y, &app.settings.visualGlowColour); y += rowH;
        simpleRow(L"Health value", app.settings.healthText, y, &app.settings.healthColour); y += rowH;
        simpleRow(L"Distance", app.settings.distance, y, &app.settings.visualGlowColour); y += rowH;
        simpleRow(L"Weapon", app.settings.weapon, y, &app.settings.damageTextColour); y += rowH + 4.0f;

        DrawLine(left, y, right, y, WithAlpha(ui.border, 0.65f));
        FillRound(Rect(left, y + 17.0f, left + 4.0f, y + 21.0f), 2.0f, ui.white);
        DrawTextValue(L"OTHER", Rect(left + 12.0f, y + 6.0f, right, y + 34.0f), app.text13.Get(), ui.white);
        y += 38.0f;
        simpleRow(L"Visible check", app.settings.visualGlow, y, &app.settings.visualGlowColour, VisualPopup::ModelOutline); y += rowH;
        simpleRow(L"Team check", app.settings.rotateModel, y, &app.settings.modelColour); y += rowH;

        app.d2dContext->PopAxisAlignedClip();

        const float contentBottom = y + app.visualScroll;
        const float visibleHeight = clipRect.bottom - clipRect.top;
        const float contentHeight = (std::max)(visibleHeight, contentBottom - (panel.top + 42.0f));
        app.visualScrollMax = (std::max)(0.0f, contentHeight - visibleHeight);
        app.visualScrollTarget = Clamp(app.visualScrollTarget, 0.0f, app.visualScrollMax);
        app.visualScroll += (app.visualScrollTarget - app.visualScroll) * 0.22f;
        if (std::fabs(app.visualScrollTarget - app.visualScroll) < 0.05f)
            app.visualScroll = app.visualScrollTarget;

        if (app.visualScrollMax > 0.5f)
        {
            const float trackTop = clipRect.top + 5.0f;
            const float trackBottom = clipRect.bottom - 5.0f;
            const float trackH = trackBottom - trackTop;
            const float thumbH = (std::max)(42.0f, trackH * (visibleHeight / contentHeight));
            const float ratio = app.visualScrollMax > 0.0f ? app.visualScroll / app.visualScrollMax : 0.0f;
            const float thumbY = trackTop + ratio * (trackH - thumbH);
            FillRound(Rect(panel.right - 6.5f, trackTop, panel.right - 5.5f, trackBottom),
                0.5f, WithAlpha(ui.white, 0.10f));
            FillRound(Rect(panel.right - 7.0f, thumbY, panel.right - 5.0f, thumbY + thumbH),
                1.0f, WithAlpha(ui.white, 0.90f));
        }
    }

    int VisualPopupIndex(VisualPopup popup)
    {
        return static_cast<int>(popup);
    }

    std::wstring VisualHotkeyText(VisualPopup popup)
    {
        const int index = VisualPopupIndex(popup);
        if (index <= 0 || index >= 7)
            return L"[Always on]";

        if (app.capturingVisualHotkey && app.hotkeyCapturePopup == popup)
            return L"[Press a key...]";

        const int vk = app.visualHotkeyVk[index];
        if (vk == 0)
            return L"[Always on]";

        UINT scanCode = MapVirtualKeyW(static_cast<UINT>(vk), MAPVK_VK_TO_VSC) << 16;
        if (vk == VK_LEFT || vk == VK_UP || vk == VK_RIGHT || vk == VK_DOWN ||
            vk == VK_PRIOR || vk == VK_NEXT || vk == VK_END || vk == VK_HOME ||
            vk == VK_INSERT || vk == VK_DELETE || vk == VK_DIVIDE || vk == VK_NUMLOCK)
        {
            scanCode |= 1u << 24;
        }

        wchar_t keyName[64]{};
        if (GetKeyNameTextW(static_cast<LONG>(scanCode), keyName, 64) == 0)
            swprintf_s(keyName, L"Key %d", vk);

        std::wstring text = L"[";
        if (app.visualHotkeyCtrl[index])
            text += L"Ctrl + ";
        text += keyName;
        text += L"]";
        return text;
    }

    void DrawVisualHotkeyRow(VisualPopup popup, float x, float y, float w)
    {
        DrawLine(x, y - 7.0f, x + w, y - 7.0f, MakeColour(0x17191D, 0.8f));
        DrawTextValue(L"Hotkey", Rect(x, y, x + 84.0f, y + 28.0f),
            app.text12.Get(), ui.text);

        const D2D1_RECT_F button = Rect(x + 86.0f, y, x + w, y + 28.0f);
        RegisterInteractive(button);
        const bool hovered = Hit(static_cast<float>(app.mouse.x),
            static_cast<float>(app.mouse.y), button);
        FillRound(button, 1.0f, hovered ? MakeColour(0x171A20, 0.98f) : MakeColour(0x0D0F14, 0.98f));

        const std::wstring value = VisualHotkeyText(popup);
        DrawTextValue(value.c_str(), button, app.text12.Get(),
            app.capturingVisualHotkey && app.hotkeyCapturePopup == popup ? ui.white : ui.muted,
            DWRITE_TEXT_ALIGNMENT_CENTER);

        if (hovered && app.clicked && !app.clickConsumed)
        {
            app.capturingVisualHotkey = true;
            app.hotkeyCapturePopup = popup;
            app.clickConsumed = true;
        }
    }

    void DrawVisualSettingsPopup(const D2D1_RECT_F& panel)
    {
        if (app.visualPopup == VisualPopup::None || app.settingsOpen)
        {
            app.visualPopupRect = Rect(0, 0, 0, 0);
            return;
        }

        float cardHeight = 150.0f;
        const wchar_t* title = L"Visual settings";

        switch (app.visualPopup)
        {
        case VisualPopup::Box:
            title = L"Box settings";
            cardHeight = 266.0f;
            break;
        case VisualPopup::HealthBar:
            title = L"Health bar settings";
            cardHeight = 342.0f;
            break;
        case VisualPopup::Snapline:
            title = L"Snap line settings";
            cardHeight = 197.0f;
            break;
        case VisualPopup::Skeleton:
            title = L"Skeleton settings";
            cardHeight = 157.0f;
            break;
        case VisualPopup::SoundWalk:
            title = L"Sound walk settings";
            cardHeight = SoundWalkUI::PopupHeight + 42.0f;
            break;
        case VisualPopup::ModelOutline:
            title = L"3D outline settings";
            cardHeight = 157.0f;
            break;
        default:
            break;
        }

        const float cardWidth = 300.0f;
        const float minLeft = 8.0f;
        const float maxLeft =
            (std::max)(minLeft, static_cast<float>(app.width) - cardWidth - 8.0f);
        const float minTop = TOP_H + 8.0f;
        const float maxTop =
            (std::max)(minTop, static_cast<float>(app.height) - cardHeight - 8.0f);

        if (!app.visualPopupPositioned)
        {
            app.visualPopupX = Clamp(panel.right + 12.0f, minLeft, maxLeft);
            app.visualPopupY = Clamp(app.visualPopupAnchorY - 8.0f, minTop, maxTop);
            app.visualPopupPositioned = true;
        }

        app.visualPopupX = Clamp(app.visualPopupX, minLeft, maxLeft);
        app.visualPopupY = Clamp(app.visualPopupY, minTop, maxTop);
        app.visualPopupRect = Rect(
            app.visualPopupX,
            app.visualPopupY,
            app.visualPopupX + cardWidth,
            app.visualPopupY + cardHeight);

        // Match the main window: almost-square 1 px corners and subtle transparency.
        FillRound(app.visualPopupRect, 1.0f, MakeColour(0x020304, 0.98f));

        DrawTextValue(
            title,
            Rect(
                app.visualPopupRect.left + 14.0f,
                app.visualPopupRect.top + 7.0f,
                app.visualPopupRect.right - 44.0f,
                app.visualPopupRect.top + 38.0f),
            app.text14.Get(),
            ui.text);

        const D2D1_RECT_F closeRect = Rect(
            app.visualPopupRect.right - 38.0f,
            app.visualPopupRect.top + 6.0f,
            app.visualPopupRect.right - 8.0f,
            app.visualPopupRect.top + 36.0f);
        RegisterInteractive(closeRect);
        RegisterInteractive(Rect(app.visualPopupRect.left, app.visualPopupRect.top,
            app.visualPopupRect.right - 42.0f, app.visualPopupRect.top + 39.0f));
        const bool closeHovered = Hit(
            static_cast<float>(app.mouse.x),
            static_cast<float>(app.mouse.y),
            closeRect);

        if (closeHovered)
            FillRound(closeRect, 1.0f, WithAlpha(ui.hover, 0.65f));

        DrawTextValue(
            L"\xE711",
            closeRect,
            app.iconFont.Get(),
            closeHovered ? ui.white : ui.muted,
            DWRITE_TEXT_ALIGNMENT_CENTER);

        if (closeHovered && app.clicked && !app.clickConsumed)
        {
            app.visualPopup = VisualPopup::None;
            app.draggingVisualPopup = false;
            app.healthStyleDropdownOpen = false;
            app.popupDropdown = PopupDropdown::None;
            app.activeSlider = SliderId::None;
            app.capturingVisualHotkey = false;
            app.hotkeyCapturePopup = VisualPopup::None;
            app.clickConsumed = true;
            return;
        }

        const float x = app.visualPopupRect.left + 15.0f;
        const float y = app.visualPopupRect.top + 45.0f;
        const float w = app.visualPopupRect.right - app.visualPopupRect.left - 30.0f;

        switch (app.visualPopup)
        {
        case VisualPopup::Box:
            PopupDropdownControl(L"Style", PopupDropdown::BoxStyle, x, y, w);
            SliderControl(
                L"Thickness",
                SliderId::BoxThickness,
                app.settings.boxThickness,
                0.6f,
                6.0f,
                x,
                y + 43.0f,
                w,
                1);
            SliderControl(
                L"Fill alpha",
                SliderId::BoxFillAlpha,
                app.settings.boxFillAlpha,
                0.0f,
                1.0f,
                x,
                y + 82.0f,
                w,
                2);
            DrawLine(
                x,
                y + 119.0f,
                x + w,
                y + 119.0f,
                MakeColour(0x17191D, 0.8f));
            ColourRow(
                L"Fill colour",
                app.settings.boxFillColour,
                x,
                y + 129.0f,
                w);
            break;

        case VisualPopup::HealthBar:
            PopupDropdownControl(L"Style", PopupDropdown::HealthStyle, x, y, w);
            SliderControl(
                L"Bar width",
                SliderId::HealthBarThickness,
                app.settings.healthBarThickness,
                2.0f,
                16.0f,
                x,
                y + 43.0f,
                w,
                1);
            SliderControl(
                L"Outer line",
                SliderId::HealthOutlineThickness,
                app.settings.healthOutlineThickness,
                0.0f,
                4.0f,
                x,
                y + 82.0f,
                w,
                1);

            DrawLine(
                x,
                y + 119.0f,
                x + w,
                y + 119.0f,
                MakeColour(0x17191D, 0.8f));

            ColourRow(
                L"Foreground",
                app.settings.healthColour,
                x,
                y + 128.0f,
                w);
            ColourRow(
                L"Background",
                app.settings.healthBackgroundColour,
                x,
                y + 161.0f,
                w);
            ColourRow(
                L"Outer line",
                app.settings.healthOutlineColour,
                x,
                y + 194.0f,
                w);
            break;

        case VisualPopup::Snapline:
            PopupDropdownControl(L"Origin", PopupDropdown::SnaplineOrigin, x, y, w);
            SliderControl(
                L"Thickness",
                SliderId::SnaplineThickness,
                app.settings.snaplineThickness,
                0.6f,
                6.0f,
                x,
                y + 47.0f,
                w,
                1);
            break;

        case VisualPopup::Skeleton:
            SliderControl(
                L"Thickness",
                SliderId::SkeletonThickness,
                app.settings.skeletonThickness,
                0.6f,
                5.0f,
                x,
                y + 5.0f,
                w,
                1);
            break;

        case VisualPopup::SoundWalk:
            SoundWalkUI::DrawPopupContents(x, y, w);
            break;

        case VisualPopup::ModelOutline:
            SliderControl(
                L"Outline size",
                SliderId::VisualGlowThickness,
                app.settings.visualGlowThickness,
                0.5f,
                6.0f,
                x,
                y + 5.0f,
                w,
                1);
            break;

        default:
            break;
        }

        // Same hotkey selector is available in every visual settings popup.
        DrawVisualHotkeyRow(app.visualPopup, x, app.visualPopupRect.bottom - 36.0f, w);
    }

    void DrawSettingsControls(const D2D1_RECT_F& panel)
    {
        app.visualPopup = VisualPopup::None;
        app.visualPopupRect = Rect(0, 0, 0, 0);
        app.healthStyleDropdownOpen = false;
        app.popupDropdown = PopupDropdown::None;

        DrawPanel(panel, L"Settings");
        const float x = panel.left + 12.0f;
        const float w = panel.right - panel.left - 24.0f;
        const float y = panel.top;

        DrawTextValue(L"APPEARANCE", Rect(x, y + 45, x + w, y + 69), app.text12.Get(), ui.muted);
        ColourRow(L"Accent color", app.settings.accentColour, x, y + 75.0f, w);
        ColourRow(L"Model color", app.settings.modelColour, x, y + 108.0f, w);

        DrawLine(x, y + 149.0f, x + w, y + 149.0f, MakeColour(0x17191D, 0.8f));
        DrawTextValue(L"PREVIEW", Rect(x, y + 158, x + w, y + 182), app.text12.Get(), ui.muted);

        SliderControl(L"Model scale", SliderId::ModelScale,
            app.settings.modelScale, 0.55f, 1.05f, x, y + 185.0f, w, 2);
        ToggleRow(L"Auto rotate", app.settings.rotateModel, x, y + 232.0f, w);
        ToggleRow(L"Background particles", app.particlesEnabled, x, y + 263.0f, w);

        DrawLine(x, y + 304.0f, x + w, y + 304.0f, MakeColour(0x17191D, 0.8f));
        SliderControl(L"Particle speed", SliderId::ParticleSpeed,
            app.particleSpeed, 4.0f, 60.0f, x, y + 318.0f, w, 0);
        SliderControl(L"Particle amount", SliderId::ParticleAmount,
            app.particleAmount, 8.0f, 70.0f, x, y + 360.0f, w, 0);
    }

