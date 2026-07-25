


// Main frame rendering order. Change which screens and overlays are drawn here.
// Included once through ProjectAll.h. Do not add this file to Source Files.


    void DrawColourPicker()
    {
        if (!app.colourPickerOpen || !app.colourTarget)
            return;

        FillRect(
            Rect(0, 0, static_cast<float>(app.width), static_cast<float>(app.height)),
            MakeColour(0x000000, 0.76f));

        const float boxWidth = 292;
        const float boxHeight = 224;
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
        const float paletteW = 208;
        const float paletteH = 108;
        const float hueX = paletteX + paletteW + 12;
        const float hueW = 16;

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
            Rect(x + 18, y + 176, x + 46, y + 198);

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
            Rect(x + 56, y + 172, x + 150, y + 202),
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
            app.saveStatusTimer = (std::max)(0.0f, app.saveStatusTimer - 0.016f);
        if (app.configStatusTimer > 0.0f)
            app.configStatusTimer = (std::max)(0.0f, app.configStatusTimer - 0.016f);

        app.interactiveRects.clear();
        UpdateHealthAnimation(0.016f);

        if (app.settings.rotateModel && !app.draggingModel && !app.colourPickerOpen)
            app.modelYaw += 0.0045f;

        const float clearColour[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
        app.d3dContext->OMSetRenderTargets(1, app.renderTargetView.GetAddressOf(), app.depthView.Get());
        app.d3dContext->ClearRenderTargetView(app.renderTargetView.Get(), clearColour);
        app.d3dContext->ClearDepthStencilView(app.depthView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        const float contentTop = TOP_H + 10.0f;
        const float contentBottom = static_cast<float>(app.height) - 12.0f;
        const D2D1_RECT_F leftPanel = Rect(14.0f, contentTop, 14.0f + LEFT_W, contentBottom);
        const D2D1_RECT_F rightPanel = Rect(leftPanel.right + GAP, contentTop,
            static_cast<float>(app.width) - 14.0f, contentBottom);

        app.sliders.clear();
        app.d2dContext->BeginDraw();
        app.d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

        // Always keep the normal menu visible. The save/config UI is now a compact popup card.
        DrawParticles(static_cast<float>(app.width), static_cast<float>(app.height));
        DrawTopBar();

        // The visual settings card is modal over the UI beneath it.  Keep the
        // popup itself interactive, but temporarily hide this frame's click
        // from controls rendered behind the card so a popup click can never
        // toggle a row, gear, paint button, or other covered control.
        const bool savedClicked = app.clicked;
        const bool clickInsideVisualPopup =
            app.visualPopup != VisualPopup::None &&
            Hit(static_cast<float>(app.mouse.x),
                static_cast<float>(app.mouse.y),
                app.visualPopupRect);

        if (clickInsideVisualPopup)
            app.clicked = false;

        if (app.settingsOpen)
            DrawSettingsControls(leftPanel);
        else
            DrawVisualControls(leftPanel);

        // Restore the click before the popup overlay is rendered below, so its
        // own sliders, dropdowns, hotkey field and close button still work.
        app.clicked = savedClicked;
        SetPreviewBounds(rightPanel);

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
            if (app.configOpen)
                DrawConfigPanel();
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


// ===== END UI/VisualsUI.hpp =====
