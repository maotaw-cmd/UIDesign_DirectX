#pragma once

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
        DrawLine(x + 2.0f, y + 21.0f, x + 12.5f, y + 3.0f, ui.muted, 1.7f);
        DrawLine(x + 12.5f, y + 3.0f, x + 23.0f, y + 21.0f, ui.muted, 1.7f);
        DrawLine(x + 23.0f, y + 21.0f, x + 2.0f, y + 21.0f, ui.muted, 1.7f);
        const D2D1_COLOR_F inner = MakeColour(0x5E636A);
        DrawLine(x + 7.0f, y + 18.0f, x + 12.5f, y + 9.0f, inner, 1.2f);
        DrawLine(x + 12.5f, y + 9.0f, x + 18.0f, y + 18.0f, inner, 1.2f);
        DrawLine(x + 18.0f, y + 18.0f, x + 7.0f, y + 18.0f, inner, 1.2f);
    }

    D2D1_RECT_F SaveButtonRect()
    {
        const float gearL = static_cast<float>(app.width) - 12.0f - 44.0f;
        return Rect(gearL - 8.0f - 44.0f, 10.0f, gearL - 8.0f, 54.0f);
    }

    D2D1_RECT_F GearButtonRect()
    {
        const float gearL = static_cast<float>(app.width) - 12.0f - 44.0f;
        return Rect(gearL, 10.0f, gearL + 44.0f, 54.0f);
    }

    void DrawTopBar()
    {
        FillRect(Rect(0, 0, static_cast<float>(app.width), TOP_H), ui.top);
        DrawTriangleLogo(24.0f, 23.0f);

        const D2D1_RECT_F saveRect = SaveButtonRect();
        const D2D1_RECT_F gearRect = GearButtonRect();
        RegisterInteractive(saveRect);
        RegisterInteractive(gearRect);
        const bool saveHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), saveRect);
        const bool gearHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), gearRect);

        // Active pages use icon colour only; selected buttons do not get a filled background.
        if (saveHover && !app.configOpen)
            FillRound(saveRect, 9.0f, WithAlpha(ui.hover, 0.55f));
        if (gearHover && !app.settingsOpen)
            FillRound(gearRect, 9.0f, WithAlpha(ui.hover, 0.55f));

        // Save/config icon opens the named configuration page.
        DrawIcon(L"\xE74E", Rect(saveRect.left + 7, saveRect.top + 7, saveRect.right - 7, saveRect.bottom - 7),
            app.configOpen ? MakeColour(0x65E592) : saveHover ? ui.text : ui.muted);
        DrawIcon(L"\xE713", Rect(gearRect.left + 7, gearRect.top + 7, gearRect.right - 7, gearRect.bottom - 7),
            app.settingsOpen ? AccentBright() : gearHover ? ui.text : ui.muted);

        if (app.saveStatusTimer > 0.0f)
            DrawTextValue(app.saveStatus, Rect(saveRect.left - 80, 10, saveRect.left - 8, 54),
                app.text12.Get(), AccentBright(), DWRITE_TEXT_ALIGNMENT_TRAILING);
    }

    void DrawParticles(float width, float height)
    {
        if (!app.particlesEnabled)
            return;

        const float time = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
        const int count = std::clamp(static_cast<int>(std::round(app.particleAmount)), 8, 70);
        const float contentTop = TOP_H + 6.0f;
        const float contentHeight = std::max(1.0f, height - contentTop - 8.0f);
        const float contentWidth = std::max(1.0f, width - 18.0f);

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
            SetBrush(MakeColour(0xFFFFFF, alpha));
            app.d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), app.brush.Get());
        }
    }

    void DrawVisualControls(const D2D1_RECT_F& panel)
    {
        DrawPanel(panel, L"Visuals");
        const float x = panel.left + 10.0f;
        const float w = panel.right - panel.left - 20.0f;
        const float columnGap = 22.0f;
        const float half = (w - columnGap) * 0.5f;
        const float right = x + half + columnGap;
        const float y = panel.top;
        const float firstRow = y + 48.0f;
        const float rowStep = 40.0f;

        ToggleColourRow(
            L"Box",
            app.settings.box,
            app.settings.boxColour,
            x,
            firstRow,
            half,
            VisualPopup::Box);

        ToggleColourRow(
            L"Health bar",
            app.settings.healthBar,
            app.settings.healthColour,
            x,
            firstRow + rowStep,
            half,
            VisualPopup::HealthBar);

        ToggleRow(L"Health text", app.settings.healthText,
            x, firstRow + rowStep * 2.0f, half);

        ToggleColourRow(
            L"Damage text",
            app.settings.damageText,
            app.settings.damageTextColour,
            x,
            firstRow + rowStep * 3.0f,
            half);

        ToggleRow(L"Name", app.settings.name,
            x, firstRow + rowStep * 4.0f, half);

        // Dedicated row: gear is always visible immediately left of the paint icon.
        SoundWalkUI::DrawRow(
            x,
            firstRow + rowStep * 5.0f,
            half);

        ToggleRow(L"Distance", app.settings.distance,
            right, firstRow, half);

        ToggleColourRow(
            L"Snap line",
            app.settings.snapline,
            app.settings.snaplineColour,
            right,
            firstRow + rowStep,
            half,
            VisualPopup::Snapline);

        ToggleColourRow(
            L"Skeleton",
            app.settings.skeleton,
            app.settings.skeletonColour,
            right,
            firstRow + rowStep * 2.0f,
            half,
            VisualPopup::Skeleton);

        ToggleColourRow(
            L"3D outline",
            app.settings.visualGlow,
            app.settings.visualGlowColour,
            right,
            firstRow + rowStep * 3.0f,
            half,
            VisualPopup::ModelOutline);

        ToggleColourRow(
            L"Sound marker",
            app.settings.soundMarker,
            app.settings.soundMarkerColour,
            right,
            firstRow + rowStep * 4.0f,
            half);

        const float dividerY = firstRow + rowStep * 6.0f + 7.0f;
        DrawLine(x, dividerY, x + w, dividerY, MakeColour(0x17191D, 0.8f));

        DrawTextValue(
            L"Use the gear beside a paint icon for style and thickness.",
            Rect(x, dividerY + 10.0f, x + w, dividerY + 38.0f),
            app.text12.Get(),
            MakeColour(0x6B7078));

        DrawTextValue(
            L"Health damage, sound rings and damage text animate automatically.",
            Rect(x, dividerY + 38.0f, x + w, dividerY + 66.0f),
            app.text12.Get(),
            MakeColour(0x5E636A));
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
            cardHeight = 224.0f;
            break;
        case VisualPopup::HealthBar:
            title = L"Health bar settings";
            cardHeight = 300.0f;
            break;
        case VisualPopup::Snapline:
            title = L"Snap line settings";
            cardHeight = 155.0f;
            break;
        case VisualPopup::Skeleton:
            title = L"Skeleton settings";
            cardHeight = 115.0f;
            break;
        case VisualPopup::SoundWalk:
            title = L"Sound walk settings";
            cardHeight = SoundWalkUI::PopupHeight;
            break;
        case VisualPopup::ModelOutline:
            title = L"3D outline settings";
            cardHeight = 115.0f;
            break;
        default:
            break;
        }

        const float cardWidth = 300.0f;
        const float minLeft = 8.0f;
        const float maxLeft =
            std::max(minLeft, static_cast<float>(app.width) - cardWidth - 8.0f);
        const float minTop = TOP_H + 8.0f;
        const float maxTop =
            std::max(minTop, static_cast<float>(app.height) - cardHeight - 8.0f);

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
        FillRound(app.visualPopupRect, 1.0f, MakeColour(0x020304, 0.985f));
        DrawRound(
            app.visualPopupRect,
            1.0f,
            MakeColour(0x0C0E12, 0.98f),
            1.0f);

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

    void SetPreviewBounds(const D2D1_RECT_F& panel)
    {
        // No card fill, title or border: the character sits directly on the window.
        app.previewRect = Rect(
            panel.left + 8.0f,
            panel.top + 2.0f,
            panel.right - 8.0f,
            panel.bottom - 4.0f);
    }

    void DrawCornerBox(const D2D1_RECT_F& box, const D2D1_COLOR_F& colour, float thickness)
    {
        const float cornerW = (box.right - box.left) * 0.22f;
        const float cornerH = (box.bottom - box.top) * 0.16f;

        // Top left
        DrawLine(box.left, box.top, box.left + cornerW, box.top, colour, thickness);
        DrawLine(box.left, box.top, box.left, box.top + cornerH, colour, thickness);

        // Top right
        DrawLine(box.right - cornerW, box.top, box.right, box.top, colour, thickness);
        DrawLine(box.right, box.top, box.right, box.top + cornerH, colour, thickness);

        // Bottom left
        DrawLine(box.left, box.bottom - cornerH, box.left, box.bottom, colour, thickness);
        DrawLine(box.left, box.bottom, box.left + cornerW, box.bottom, colour, thickness);

        // Bottom right
        DrawLine(box.right, box.bottom - cornerH, box.right, box.bottom, colour, thickness);
        DrawLine(box.right - cornerW, box.bottom, box.right, box.bottom, colour, thickness);
    }

    void UpdateHealthAnimation(float deltaSeconds)
    {
        static constexpr std::array<float, 5> damageTargets{ 84.0f, 63.0f, 42.0f, 21.0f, 0.0f };

        // Smoothly animate only the falling health value, like repeated damage hits.
        if (app.previewHealth > app.previewHealthTarget)
        {
            app.previewHealth = std::max(
                app.previewHealthTarget,
                app.previewHealth - 54.0f * deltaSeconds);
        }

        if (app.damageTextTimer > 0.0f)
            app.damageTextTimer = std::max(0.0f, app.damageTextTimer - deltaSeconds);

        app.healthAnimationTimer -= deltaSeconds;
        if (app.healthAnimationTimer > 0.0f)
            return;

        if (app.healthDamageStep < static_cast<int>(damageTargets.size()))
        {
            const float previousTarget = app.previewHealthTarget;
            app.previewHealthTarget = damageTargets[app.healthDamageStep++];
            app.healthAnimationTimer = 0.82f;
            app.damageTextValue = std::max(0, static_cast<int>(std::round(previousTarget - app.previewHealthTarget)));
            if (app.damageTextValue > 0)
                app.damageTextTimer = 0.92f;
        }
        else
        {
            // Hold briefly at zero, then restart the damage showcase at full health.
            app.previewHealth = 100.0f;
            app.previewHealthTarget = 100.0f;
            app.healthDamageStep = 0;
            app.healthAnimationTimer = 1.10f;
        }
    }

    void DrawPreviewOverlay()
    {
        const D2D1_RECT_F bounds = app.previewRect;

        const float centreX = (bounds.left + bounds.right) * 0.5f;

        // ESP preview geometry is fixed. Model scale only changes the 3D character;
        // the box, health bar, name, distance and snap line keep their size/position.
        const float overlayHeight = (bounds.bottom - bounds.top) * 0.76f;
        const float overlayWidth = overlayHeight * 0.34f;
        const float overlayTop =
            bounds.top + (bounds.bottom - bounds.top - overlayHeight) * 0.5f;

        const D2D1_RECT_F box =
            Rect(
                centreX - overlayWidth * 0.72f,
                overlayTop,
                centreX + overlayWidth * 0.72f,
                overlayTop + overlayHeight);

        if (app.settings.box)
        {
            const int boxStyle = std::clamp(app.settings.boxStyle, 0, 4);
            const bool filledBox = boxStyle == 3 || boxStyle == 4;

            if (filledBox)
            {
                D2D1_COLOR_F fill = app.settings.boxFillColour;
                fill.a = Clamp(app.settings.boxFillAlpha, 0.0f, 1.0f);

                // Keep the fill slightly inside for Filled + corners so the
                // outer corner strokes remain clearly readable.
                const float fillInset = boxStyle == 4
                    ? std::max(5.0f, app.settings.boxThickness * 2.2f)
                    : 0.0f;
                const D2D1_RECT_F fillRect = Rect(
                    box.left + fillInset,
                    box.top + fillInset,
                    box.right - fillInset,
                    box.bottom - fillInset);

                if (fillRect.right > fillRect.left && fillRect.bottom > fillRect.top)
                    FillRect(fillRect, fill);
            }

            if (boxStyle == 0 || boxStyle == 2 || boxStyle == 3 || boxStyle == 4)
            {
                DrawRect(box, app.settings.boxColour, app.settings.boxThickness);
            }

            if (boxStyle == 1 || boxStyle == 2 || boxStyle == 4)
            {
                DrawCornerBox(box, app.settings.boxColour, app.settings.boxThickness);
            }
        }

        const float healthRatioForText = Clamp(app.previewHealth / 100.0f, 0.0f, 1.0f);
        const float healthTextAnchorY =
            box.bottom - (box.bottom - box.top) * healthRatioForText;

        if (app.settings.healthBar)
        {
            const float healthRatio = healthRatioForText;
            const D2D1_COLOR_F healthColour = app.settings.healthColour;
            const D2D1_COLOR_F emptyColour = app.settings.healthBackgroundColour;
            const D2D1_COLOR_F borderColour = app.settings.healthOutlineColour;
            const int style = std::clamp(app.settings.healthBarStyle, 0, 5);
            const float barThickness =
                Clamp(app.settings.healthBarThickness, 2.0f, 16.0f);
            const float outlineThickness =
                Clamp(app.settings.healthOutlineThickness, 0.0f, 4.0f);

            auto drawBorder = [&](const D2D1_RECT_F& r)
                {
                    if (outlineThickness > 0.01f)
                        DrawRect(r, borderColour, outlineThickness);
                };

            const float inset =
                outlineThickness > 0.01f ? std::max(1.0f, outlineThickness) : 0.0f;

            if (style == 0) // Classic
            {
                const D2D1_RECT_F bar = Rect(
                    box.left - barThickness - 9.0f,
                    box.top,
                    box.left - 9.0f,
                    box.bottom);
                const float fillTop =
                    bar.bottom - (bar.bottom - bar.top) * healthRatio;

                FillRect(bar, emptyColour);
                if (healthRatio > 0.001f)
                {
                    const D2D1_RECT_F fill = Rect(
                        bar.left + inset,
                        std::max(bar.top + inset, fillTop),
                        bar.right - inset,
                        bar.bottom - inset);
                    FillRect(fill, healthColour);
                }
                drawBorder(bar);
            }
            else if (style == 1) // Thin
            {
                const float thinWidth = std::max(2.0f, barThickness * 0.48f);
                const D2D1_RECT_F bar = Rect(
                    box.left - thinWidth - 8.0f,
                    box.top,
                    box.left - 8.0f,
                    box.bottom);
                const float fillTop =
                    bar.bottom - (bar.bottom - bar.top) * healthRatio;

                FillRect(bar, emptyColour);
                if (healthRatio > 0.001f)
                {
                    const D2D1_RECT_F fill = Rect(
                        bar.left + inset,
                        std::max(bar.top + inset, fillTop),
                        bar.right - inset,
                        bar.bottom - inset);
                    FillRect(fill, healthColour);
                }
                drawBorder(bar);
            }
            else if (style == 2 || style == 3 || style == 4) // Cut/segmented variants
            {
                const int segmentCount = style == 2 ? 10 : style == 3 ? 6 : 5;
                const float segmentWidth =
                    style == 3 ? barThickness * 1.32f :
                    style == 4 ? barThickness * 0.92f :
                    barThickness;
                const float gap =
                    style == 3 ? 4.0f :
                    style == 4 ? 3.0f :
                    2.0f;
                const float barLeft = box.left - segmentWidth - 9.0f;
                const float totalHeight = box.bottom - box.top;
                const float segmentHeight =
                    (totalHeight - gap * static_cast<float>(segmentCount - 1)) /
                    static_cast<float>(segmentCount);
                const float healthSegments =
                    healthRatio * static_cast<float>(segmentCount);

                for (int index = 0; index < segmentCount; ++index)
                {
                    const float top =
                        box.top + static_cast<float>(index) * (segmentHeight + gap);
                    const D2D1_RECT_F segment = Rect(
                        barLeft,
                        top,
                        barLeft + segmentWidth,
                        top + segmentHeight);

                    FillRect(segment, emptyColour);
                    drawBorder(segment);

                    const int fromBottom = segmentCount - 1 - index;
                    const float segmentFill = Clamp(
                        healthSegments - static_cast<float>(fromBottom),
                        0.0f,
                        1.0f);

                    if (segmentFill > 0.001f)
                    {
                        const float fillTop =
                            segment.bottom -
                            (segment.bottom - segment.top) * segmentFill;
                        const D2D1_RECT_F fill = Rect(
                            segment.left + inset,
                            std::max(segment.top + inset, fillTop),
                            segment.right - inset,
                            segment.bottom - inset);
                        FillRect(fill, healthColour);
                    }
                }
            }
            else // Bottom segmented bar
            {
                constexpr int segmentCount = 12;
                constexpr float gap = 2.0f;
                const float totalWidth = box.right - box.left;
                const float segmentWidth =
                    (totalWidth - gap * static_cast<float>(segmentCount - 1)) /
                    static_cast<float>(segmentCount);
                const float healthSegments =
                    healthRatio * static_cast<float>(segmentCount);
                const float top = box.bottom + 9.0f;
                const float height = std::max(3.0f, barThickness * 0.62f);

                for (int index = 0; index < segmentCount; ++index)
                {
                    const float left =
                        box.left + static_cast<float>(index) * (segmentWidth + gap);
                    const D2D1_RECT_F segment =
                        Rect(left, top, left + segmentWidth, top + height);

                    FillRect(segment, emptyColour);
                    drawBorder(segment);

                    const float segmentFill = Clamp(
                        healthSegments - static_cast<float>(index),
                        0.0f,
                        1.0f);

                    if (segmentFill > 0.001f)
                    {
                        const float available =
                            std::max(0.0f, segment.right - segment.left - inset * 2.0f);
                        const D2D1_RECT_F fill = Rect(
                            segment.left + inset,
                            segment.top + inset,
                            segment.left + inset + available * segmentFill,
                            segment.bottom - inset);
                        FillRect(fill, healthColour);
                    }
                }
            }
        }

        if (app.settings.healthText)
        {
            wchar_t healthText[24]{};
            swprintf_s(healthText, L"%.0f", app.previewHealth);

            const float textTop = Clamp(
                healthTextAnchorY - 11.0f,
                box.top - 4.0f,
                box.bottom - 18.0f);

            DrawTextValue(
                healthText,
                Rect(box.left - 70.0f, textTop, box.left - 17.0f, textTop + 24.0f),
                app.text12.Get(),
                ui.text,
                DWRITE_TEXT_ALIGNMENT_TRAILING);
        }

        if (app.settings.damageText && app.damageTextTimer > 0.0f)
        {
            constexpr float life = 0.92f;
            const float progress = Clamp(1.0f - (app.damageTextTimer / life), 0.0f, 1.0f);

            // Fast scale-up with a small overshoot, then smoothly settle to normal size.
            float scale = 1.0f;
            if (progress < 0.24f)
            {
                const float t = Clamp(progress / 0.24f, 0.0f, 1.0f);
                const float eased = 1.0f - std::pow(1.0f - t, 3.0f);
                scale = 0.55f + eased * 0.76f;
            }
            else
            {
                const float t = Clamp((progress - 0.24f) / 0.30f, 0.0f, 1.0f);
                const float eased = t * t * (3.0f - 2.0f * t);
                scale = 1.31f - eased * 0.31f;
            }

            const float rise = progress * 58.0f + std::sin(progress * 3.14159265f) * 5.0f;
            const float alpha = progress < 0.62f
                ? 1.0f
                : Clamp(1.0f - (progress - 0.62f) / 0.38f, 0.0f, 1.0f);

            wchar_t damageText[24]{};
            swprintf_s(damageText, L"-%d", std::max(0, app.damageTextValue));

            // Kept close to the health bar while leaving a small readable gap.
            const D2D1_RECT_F damageRect = Rect(
                box.left - 126.0f,
                box.top + 50.0f - rise,
                box.left - 24.0f,
                box.top + 88.0f - rise);
            const float centreX = (damageRect.left + damageRect.right) * 0.5f;
            const float centreY = (damageRect.top + damageRect.bottom) * 0.5f;

            D2D1_MATRIX_3X2_F previousTransform{};
            app.d2dContext->GetTransform(&previousTransform);
            app.d2dContext->SetTransform(
                D2D1::Matrix3x2F::Scale(
                    D2D1::SizeF(scale, scale),
                    D2D1::Point2F(centreX, centreY)));

            const D2D1_COLOR_F shadowColour = MakeColour(0x120507, alpha * 0.92f);
            D2D1_COLOR_F damageColour = app.settings.damageTextColour;
            damageColour.a = alpha;
            const D2D1_COLOR_F highlightColour = D2D1::ColorF(
                Clamp(damageColour.r * 0.55f + 0.45f, 0.0f, 1.0f),
                Clamp(damageColour.g * 0.55f + 0.45f, 0.0f, 1.0f),
                Clamp(damageColour.b * 0.55f + 0.45f, 0.0f, 1.0f),
                alpha * 0.75f);

            // Four dark offset passes make a clean game-like text outline.
            DrawTextValue(damageText, Rect(damageRect.left - 2.0f, damageRect.top,
                damageRect.right - 2.0f, damageRect.bottom),
                app.text27Bold.Get(), shadowColour, DWRITE_TEXT_ALIGNMENT_CENTER);
            DrawTextValue(damageText, Rect(damageRect.left + 2.0f, damageRect.top,
                damageRect.right + 2.0f, damageRect.bottom),
                app.text27Bold.Get(), shadowColour, DWRITE_TEXT_ALIGNMENT_CENTER);
            DrawTextValue(damageText, Rect(damageRect.left, damageRect.top - 2.0f,
                damageRect.right, damageRect.bottom - 2.0f),
                app.text27Bold.Get(), shadowColour, DWRITE_TEXT_ALIGNMENT_CENTER);
            DrawTextValue(damageText, Rect(damageRect.left, damageRect.top + 2.0f,
                damageRect.right, damageRect.bottom + 2.0f),
                app.text27Bold.Get(), shadowColour, DWRITE_TEXT_ALIGNMENT_CENTER);

            DrawTextValue(damageText, damageRect, app.text27Bold.Get(),
                damageColour, DWRITE_TEXT_ALIGNMENT_CENTER);
            DrawTextValue(damageText,
                Rect(damageRect.left, damageRect.top - 1.0f,
                    damageRect.right, damageRect.bottom - 1.0f),
                app.text27Bold.Get(), highlightColour,
                DWRITE_TEXT_ALIGNMENT_CENTER);

            app.d2dContext->SetTransform(previousTransform);
        }

        if (app.settings.name)
        {
            DrawTextValue(
                L"PLAYER",
                Rect(box.left - 20, box.top - 27, box.right + 20, box.top - 5),
                app.text13.Get(),
                ui.white,
                DWRITE_TEXT_ALIGNMENT_CENTER);
        }

        if (app.settings.distance)
        {
            const bool bottomHealth =
                app.settings.healthBar && app.settings.healthBarStyle == 5;
            const float distanceTop = box.bottom + (bottomHealth ? 19.0f : 3.0f);

            DrawTextValue(
                L"18 m",
                Rect(box.left - 10, distanceTop, box.right + 10, distanceTop + 22.0f),
                app.text12.Get(),
                ui.muted,
                DWRITE_TEXT_ALIGNMENT_CENTER);
        }

        if (app.settings.snapline)
        {
            const bool bottomHealth =
                app.settings.healthBar && app.settings.healthBarStyle == 5;
            const float snapEndY = box.bottom + (bottomHealth ? 17.0f : 0.0f);
            const int origin = std::clamp(app.settings.snaplineOrigin, 0, 2);
            const float snapStartY =
                origin == 0
                ? bounds.bottom
                : origin == 1
                ? (bounds.top + bounds.bottom) * 0.5f
                : bounds.top;
            const float thickness =
                Clamp(app.settings.snaplineThickness, 0.6f, 6.0f);

            DrawLine(
                centreX,
                snapStartY,
                centreX,
                snapEndY,
                app.settings.snaplineColour,
                thickness);
        }

        const float groundY = box.bottom + 4.0f;
        const float pulseTime = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
        if (app.settings.soundWalk)
        {
            const int animationStyle = std::clamp(app.settings.soundWalkAnimationStyle, 0, 3);
            const float speed = Clamp(app.settings.soundWalkSpeed, 0.35f, 1.80f);
            const float expansion = Clamp(app.settings.soundWalkExpansion, 0.70f, 2.30f);
            const float lineWidth = Clamp(app.settings.soundWalkThickness, 0.60f, 4.00f);

            float duration = 1.50f;
            float maximumRadius = 48.0f;
            float maximumAlpha = 0.58f;
            int ringCount = 2;

            switch (animationStyle)
            {
            case 1: // Cinematic: slower, wider, softer and layered.
                duration = 1.95f;
                maximumRadius = 62.0f;
                maximumAlpha = 0.64f;
                ringCount = 3;
                break;
            case 2: // Fast: compact responsive pulses.
                duration = 0.92f;
                maximumRadius = 43.0f;
                maximumAlpha = 0.72f;
                ringCount = 2;
                break;
            case 3: // Minimal: one subtle ring.
                duration = 1.38f;
                maximumRadius = 42.0f;
                maximumAlpha = 0.42f;
                ringCount = 1;
                break;
            default: // Smooth
                break;
            }

            duration /= speed;
            maximumRadius *= expansion;

            for (int ringIndex = 0; ringIndex < ringCount; ++ringIndex)
            {
                const float stagger = duration / static_cast<float>(ringCount);
                const float phase = std::fmod(
                    pulseTime + static_cast<float>(ringIndex) * stagger,
                    duration) / duration;

                // Smoothstep expansion avoids the mechanical linear look.
                const float eased = phase * phase * (3.0f - 2.0f * phase);
                const float fade = (1.0f - phase) * (1.0f - phase);
                const float rx = 13.0f + eased * maximumRadius;
                const float ry = 4.0f + eased * maximumRadius * 0.30f;
                const float alpha = fade * maximumAlpha;

                SetBrush(WithAlpha(app.settings.soundWalkColour, alpha));
                app.d2dContext->DrawEllipse(
                    D2D1::Ellipse(D2D1::Point2F(centreX, groundY), rx, ry),
                    app.brush.Get(),
                    lineWidth);
            }
        }

        if (app.settings.soundMarker)
        {
            const D2D1_COLOR_F markerFill = WithAlpha(app.settings.soundMarkerColour, 0.22f);
            const D2D1_COLOR_F markerLine = WithAlpha(app.settings.soundMarkerColour, 0.92f);
            SetBrush(markerFill);
            app.d2dContext->FillEllipse(
                D2D1::Ellipse(D2D1::Point2F(centreX, groundY), 12.0f, 4.5f),
                app.brush.Get());
            SetBrush(markerLine);
            app.d2dContext->DrawEllipse(
                D2D1::Ellipse(D2D1::Point2F(centreX, groundY), 12.0f, 4.5f),
                app.brush.Get(),
                1.2f);
        }

        // Skeleton is intentionally not drawn over the 3D character preview.
    }

    void DrawColourPicker()
    {
        if (!app.colourPickerOpen || !app.colourTarget)
            return;

        FillRect(
            Rect(0, 0, static_cast<float>(app.width), static_cast<float>(app.height)),
            MakeColour(0x000000, 0.76f));

        const float boxWidth = 370;
        const float boxHeight = 310;
        const float x = (static_cast<float>(app.width) - boxWidth) * 0.5f;
        const float y = (static_cast<float>(app.height) - boxHeight) * 0.5f;

        FillRound(Rect(x, y, x + boxWidth, y + boxHeight), 6, MakeColour(0x0D0F14));
        DrawRound(Rect(x, y, x + boxWidth, y + boxHeight), 6, app.settings.accentColour);

        DrawTextValue(
            L"Colour picker",
            Rect(x + 16, y + 4, x + boxWidth - 16, y + 40),
            app.text16Bold.Get(),
            ui.text);

        DrawLine(x + 14, y + 42, x + boxWidth - 14, y + 42, ui.border);

        const float paletteX = x + 18;
        const float paletteY = y + 58;
        const float paletteW = 280;
        const float paletteH = 180;
        const float hueX = paletteX + paletteW + 12;
        const float hueW = 20;

        constexpr int columns = 70;
        constexpr int rows = 45;

        const float cellW = paletteW / columns;
        const float cellH = paletteH / rows;

        for (int row = 0; row < rows; ++row)
        {
            const float value = 1.0f -
                static_cast<float>(row) / static_cast<float>(rows - 1);

            for (int column = 0; column < columns; ++column)
            {
                const float saturation =
                    static_cast<float>(column) /
                    static_cast<float>(columns - 1);

                const float left = paletteX + cellW * column;
                const float top = paletteY + cellH * row;

                FillRect(
                    Rect(left, top, left + cellW + 0.7f, top + cellH + 0.7f),
                    HsvToColour(app.pickerHue, saturation, value));
            }
        }

        constexpr int hueSteps = 90;
        const float stepH = paletteH / hueSteps;

        for (int step = 0; step < hueSteps; ++step)
        {
            const float hue =
                static_cast<float>(step) /
                static_cast<float>(hueSteps - 1);

            const float top = paletteY + stepH * step;

            FillRect(
                Rect(hueX, top, hueX + hueW, top + stepH + 0.7f),
                HsvToColour(hue, 1, 1));
        }

        const D2D1_RECT_F paletteRect =
            Rect(paletteX, paletteY, paletteX + paletteW, paletteY + paletteH);

        const D2D1_RECT_F hueRect =
            Rect(hueX, paletteY, hueX + hueW, paletteY + paletteH);

        const bool paletteHover =
            Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), paletteRect);

        const bool hueHover =
            Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), hueRect);

        if (app.clicked && paletteHover)
        {
            app.activePickerArea = 0;
            app.clickConsumed = true;
        }
        else if (app.clicked && hueHover)
        {
            app.activePickerArea = 1;
            app.clickConsumed = true;
        }

        if (app.mouseDown && app.activePickerArea == 0)
        {
            app.pickerSaturation = Clamp(
                (static_cast<float>(app.mouse.x) - paletteX) / paletteW,
                0,
                1);

            app.pickerValue = 1.0f - Clamp(
                (static_cast<float>(app.mouse.y) - paletteY) / paletteH,
                0,
                1);

            app.workingColour =
                HsvToColour(app.pickerHue, app.pickerSaturation, app.pickerValue);
        }
        else if (app.mouseDown && app.activePickerArea == 1)
        {
            app.pickerHue = Clamp(
                (static_cast<float>(app.mouse.y) - paletteY) / paletteH,
                0,
                1);

            app.workingColour =
                HsvToColour(app.pickerHue, app.pickerSaturation, app.pickerValue);
        }

        const float markerX = paletteX + app.pickerSaturation * paletteW;
        const float markerY = paletteY + (1.0f - app.pickerValue) * paletteH;

        SetBrush(MakeColour(0x000000));
        app.d2dContext->DrawEllipse(
            D2D1::Ellipse(D2D1::Point2F(markerX, markerY), 6, 6),
            app.brush.Get(),
            3);

        SetBrush(ui.white);
        app.d2dContext->DrawEllipse(
            D2D1::Ellipse(D2D1::Point2F(markerX, markerY), 6, 6),
            app.brush.Get(),
            1.5f);

        const float hueMarkerY = paletteY + app.pickerHue * paletteH;
        FillRect(Rect(hueX - 3, hueMarkerY - 2, hueX + hueW + 3, hueMarkerY + 2), ui.white);

        const D2D1_RECT_F colourPreview =
            Rect(x + 18, y + 252, x + 54, y + 278);

        FillRound(colourPreview, 3, app.workingColour);
        DrawRound(colourPreview, 3, WithAlpha(ui.white, 0.2f));

        wchar_t hex[16]{};
        const int red = std::clamp(
            static_cast<int>(std::round(app.workingColour.r * 255)), 0, 255);
        const int green = std::clamp(
            static_cast<int>(std::round(app.workingColour.g * 255)), 0, 255);
        const int blue = std::clamp(
            static_cast<int>(std::round(app.workingColour.b * 255)), 0, 255);

        swprintf_s(hex, L"#%02X%02X%02X", red, green, blue);

        DrawTextValue(
            hex,
            Rect(x + 65, y + 249, x + 175, y + 281),
            app.text13.Get(),
            ui.text);

        const D2D1_RECT_F cancel =
            Rect(x + boxWidth - 158, y + boxHeight - 42,
                x + boxWidth - 88, y + boxHeight - 14);

        const D2D1_RECT_F apply =
            Rect(x + boxWidth - 78, y + boxHeight - 42,
                x + boxWidth - 14, y + boxHeight - 14);

        RegisterInteractive(cancel);
        RegisterInteractive(apply);
        const bool cancelHover =
            Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), cancel);

        const bool applyHover =
            Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), apply);

        FillRound(cancel, 3, cancelHover ? ui.hover : ui.row);
        DrawRound(cancel, 3, ui.border);
        DrawTextValue(L"Cancel", cancel, app.text13.Get(), ui.text, DWRITE_TEXT_ALIGNMENT_CENTER);

        FillRound(apply, 3, applyHover ? WithAlpha(app.settings.accentColour, 0.78f)
            : WithAlpha(app.settings.accentColour, 0.56f));
        DrawRound(apply, 3, app.settings.accentColour);
        DrawTextValue(L"Apply", apply, app.text13.Get(), ui.white, DWRITE_TEXT_ALIGNMENT_CENTER);

        if (app.clicked && cancelHover)
        {
            *app.colourTarget = app.originalColour;
            app.colourPickerOpen = false;
            app.colourTarget = nullptr;
            app.activePickerArea = -1;
            app.clickConsumed = true;
        }
        else if (app.clicked && applyHover)
        {
            *app.colourTarget = app.workingColour;
            app.colourPickerOpen = false;
            app.colourTarget = nullptr;
            app.activePickerArea = -1;
            app.clickConsumed = true;
        }
    }

    void Render()
    {
        if (!app.renderTargetView || !app.d2dTarget)
            return;

        if (app.saveStatusTimer > 0.0f)
            app.saveStatusTimer = std::max(0.0f, app.saveStatusTimer - 0.016f);
        if (app.configStatusTimer > 0.0f)
            app.configStatusTimer = std::max(0.0f, app.configStatusTimer - 0.016f);

        app.interactiveRects.clear();
        UpdateHealthAnimation(0.016f);

        if (app.settings.rotateModel && !app.draggingModel && !app.colourPickerOpen)
            app.modelYaw += 0.0045f;

        const float clearColour[4]{ 0.004f, 0.008f, 0.012f, 1.0f };
        app.d3dContext->OMSetRenderTargets(1, app.renderTargetView.GetAddressOf(), app.depthView.Get());
        app.d3dContext->ClearRenderTargetView(app.renderTargetView.Get(), clearColour);
        app.d3dContext->ClearDepthStencilView(app.depthView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        const float contentTop = TOP_H + 12.0f;
        const float contentBottom = static_cast<float>(app.height) - 24.0f;
        const D2D1_RECT_F leftPanel = Rect(20.0f, contentTop, 20.0f + LEFT_W, contentBottom);
        const D2D1_RECT_F rightPanel = Rect(leftPanel.right + GAP, contentTop,
            static_cast<float>(app.width) - 20.0f, contentBottom);

        app.sliders.clear();
        app.d2dContext->BeginDraw();
        app.d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

        if (app.configOpen)
        {
            // Configuration mode replaces the complete content view.
            // No particles, visual controls, character, preview overlays, or popups are drawn.
            DrawTopBar();
            DrawConfigPanel();
        }
        else
        {
            DrawParticles(static_cast<float>(app.width), static_cast<float>(app.height));
            DrawTopBar();
            if (app.settingsOpen)
                DrawSettingsControls(leftPanel);
            else
                DrawVisualControls(leftPanel);
            SetPreviewBounds(rightPanel);
        }

        DrawRound(Rect(0.5f, 0.5f, static_cast<float>(app.width) - 0.5f,
            static_cast<float>(app.height) - 0.5f),
            8.0f, MakeColour(0x15171A));

        HRESULT hr = app.d2dContext->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET)
        {
            app.d2dTarget.Reset();
            CreateSizeResources();
            return;
        }

        if (!app.configOpen)
        {
            app.d3dContext->OMSetRenderTargets(1, app.renderTargetView.GetAddressOf(), app.depthView.Get());
            app.d3dContext->ClearDepthStencilView(app.depthView.Get(),
                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            DrawMannequin(app.previewRect.left, app.previewRect.top,
                app.previewRect.right - app.previewRect.left,
                app.previewRect.bottom - app.previewRect.top);

            app.d2dContext->BeginDraw();
            app.d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
            DrawPreviewOverlay();
            if (!app.settingsOpen)
            {
                DrawVisualSettingsPopup(leftPanel);
                DrawHealthStyleDropdownOverlay();
            }
            DrawColourPicker();
            hr = app.d2dContext->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET)
            {
                app.d2dTarget.Reset();
                CreateSizeResources();
                return;
            }
        }

        UpdatePointerCursor();
        app.swapChain->Present(1, 0);
        app.clicked = false;
        app.clickConsumed = false;
    }

