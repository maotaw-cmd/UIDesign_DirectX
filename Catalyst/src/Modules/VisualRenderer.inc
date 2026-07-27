


// Editable visual-preview rendering: box, corner box, health bars,
// snaplines, names, distance, damage text, sound walk and markers.
// Included once by main.cpp after the common drawing helpers.

    void SetPreviewBounds(const D2D1_RECT_F& panel)
    {
        // The live preview now uses the complete right side; the old profile strip is removed.
        const D2D1_RECT_F previewPanel = panel;

        // The preview is borderless and everything drawn inside it is hard-clipped.
        FillRect(previewPanel, ui.panel);
        app.d2dContext->PushAxisAlignedClip(previewPanel, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        FillRound(Rect(previewPanel.left + 18.0f, previewPanel.top + 20.0f, previewPanel.left + 22.0f, previewPanel.top + 24.0f),
            2.0f, ui.white);
        DrawTextValue(L"LIVE PREVIEW", Rect(previewPanel.left + 30.0f, previewPanel.top + 8.0f,
            previewPanel.right - 18.0f, previewPanel.top + 36.0f), app.text14.Get(), ui.white);

        const float t = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
        for (int i = 0; i < 18; ++i)
        {
            const float seed = static_cast<float>(i + 3);
            const float px = previewPanel.left + 22.0f + std::fmod(seed * 89.7f,
                previewPanel.right - previewPanel.left - 44.0f);
            const float py = previewPanel.top + 55.0f + std::fmod(seed * 57.2f +
                t * (2.0f + std::fmod(seed, 4.0f)), previewPanel.bottom - previewPanel.top - 80.0f);
            SetBrush(WithAlpha(app.settings.accentColour, 0.16f + std::fmod(seed * 0.07f, 0.20f)));
            app.d2dContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(px, py), 1.0f, 1.0f), app.brush.Get());
        }

        // Grid removed: keep the live preview clean and borderless.

        app.previewRect = Rect(previewPanel.left + 10.0f, previewPanel.top + 36.0f,
            previewPanel.right - 10.0f, previewPanel.bottom - 8.0f);
        app.d2dContext->PopAxisAlignedClip();
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
            app.previewHealth = (std::max)(
                app.previewHealthTarget,
                app.previewHealth - 54.0f * deltaSeconds);
        }

        if (app.damageTextTimer > 0.0f)
            app.damageTextTimer = (std::max)(0.0f, app.damageTextTimer - deltaSeconds);

        app.healthAnimationTimer -= deltaSeconds;
        if (app.healthAnimationTimer > 0.0f)
            return;

        if (app.healthDamageStep < static_cast<int>(damageTargets.size()))
        {
            const float previousTarget = app.previewHealthTarget;
            app.previewHealthTarget = damageTargets[app.healthDamageStep++];
            app.healthAnimationTimer = 0.82f;
            app.damageTextValue = (std::max)(0, static_cast<int>(std::round(previousTarget - app.previewHealthTarget)));
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
        app.d2dContext->PushAxisAlignedClip(bounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

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
            const int boxStyle = (std::clamp)(app.settings.boxStyle, 0, 4);
            const bool filledBox = boxStyle == 3 || boxStyle == 4;

            if (filledBox)
            {
                D2D1_COLOR_F fill = app.settings.boxFillColour;
                fill.a = Clamp(app.settings.boxFillAlpha, 0.0f, 1.0f);

                // Keep the fill slightly inside for Filled + corners so the
                // outer corner strokes remain clearly readable.
                const float fillInset = boxStyle == 4
                    ? (std::max)(5.0f, app.settings.boxThickness * 2.2f)
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

            // Dynamic health colours:
            //   50-100 HP = configured/default green
            //   30-49  HP = orange
            //   0-29   HP = red
            D2D1_COLOR_F healthColour = app.settings.healthColour;
            if (healthRatio < 0.30f)
                healthColour = MakeColour(0xF04444, app.settings.healthColour.a);
            else if (healthRatio < 0.50f)
                healthColour = MakeColour(0xF59E0B, app.settings.healthColour.a);

            const D2D1_COLOR_F emptyColour = app.settings.healthBackgroundColour;
            const D2D1_COLOR_F borderColour = app.settings.healthOutlineColour;
            const int style = (std::clamp)(app.settings.healthBarStyle, 0, 5);
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
                outlineThickness > 0.01f ? (std::max)(1.0f, outlineThickness) : 0.0f;

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
                        (std::max)(bar.top + inset, fillTop),
                        bar.right - inset,
                        bar.bottom - inset);
                    FillRect(fill, healthColour);
                }
                drawBorder(bar);
            }
            else if (style == 1) // Thin
            {
                const float thinWidth = (std::max)(2.0f, barThickness * 0.48f);
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
                        (std::max)(bar.top + inset, fillTop),
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
                            (std::max)(segment.top + inset, fillTop),
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
                const float height = (std::max)(3.0f, barThickness * 0.62f);

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
                            (std::max)(0.0f, segment.right - segment.left - inset * 2.0f);
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
            swprintf_s(damageText, L"-%d", (std::max)(0, app.damageTextValue));

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

        if (app.settings.weapon)
        {
            const bool bottomHealth =
                app.settings.healthBar && app.settings.healthBarStyle == 5;
            const float distanceTop = box.bottom + (bottomHealth ? 19.0f : 3.0f);
            const float weaponTop = distanceTop + (app.settings.distance ? 18.0f : 0.0f);
            const wchar_t* weaponName = app.previewWeaponName.empty()
                ? L"Unknown"
                : app.previewWeaponName.c_str();

            DrawTextValue(
                weaponName,
                Rect(box.left - 30.0f, weaponTop, box.right + 30.0f, weaponTop + 22.0f),
                app.text12.Get(),
                ui.white,
                DWRITE_TEXT_ALIGNMENT_CENTER);
        }

        if (app.settings.snapline)
        {
            const bool bottomHealth =
                app.settings.healthBar && app.settings.healthBarStyle == 5;
            const float snapEndY = box.bottom + (bottomHealth ? 17.0f : 0.0f);
            const int origin = (std::clamp)(app.settings.snaplineOrigin, 0, 2);
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
            const int animationStyle = (std::clamp)(app.settings.soundWalkAnimationStyle, 0, 3);
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
            app.d2dContext->PopAxisAlignedClip();
    }

