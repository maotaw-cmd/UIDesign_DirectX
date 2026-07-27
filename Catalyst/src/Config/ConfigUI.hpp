#pragma once

// Configuration page UI only.
// File operations are delegated to ConfigManager.hpp.

    void DrawConfigIcon(const wchar_t* icon, const D2D1_RECT_F& rect, const D2D1_COLOR_F& colour)
    {
        DrawTextValue(icon, rect, app.iconFont.Get(), colour, DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    void DrawConfigPanel()
    {
        // Full-window configuration page using the existing main-window background.
        // There is intentionally no modal dimmer, card fill, or outer card border.
        const float left = 30.0f;
        const float top = TOP_H + 14.0f;
        const float panelW = static_cast<float>(app.width) - 60.0f;
        const float panelH = static_cast<float>(app.height) - top - 18.0f;
        app.configPanelRect = Rect(0.0f, TOP_H, static_cast<float>(app.width), static_cast<float>(app.height));

        DrawTextValue(L"Configurations",
            Rect(left + 4.0f, top + 5.0f, left + 310.0f, top + 39.0f),
            app.text19Bold.Get(), ui.text);

        const D2D1_RECT_F closeRect = Rect(static_cast<float>(app.width) - 52.0f, top + 3.0f,
            static_cast<float>(app.width) - 20.0f, top + 35.0f);
        RegisterInteractive(closeRect);
        const bool closeHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), closeRect);
        if (closeHover)
            FillRound(closeRect, 1.0f, WithAlpha(ui.hover, 0.65f));
        DrawConfigIcon(L"\xE711", closeRect, closeHover ? ui.white : ui.muted);

        DrawTextValue(L"Name", Rect(left + 4.0f, top + 48.0f, left + 92.0f, top + 74.0f),
            app.text12.Get(), ui.muted);

        app.configInputRect = Rect(left + 4.0f, top + 74.0f,
            left + panelW - 70.0f, top + 110.0f);
        const D2D1_RECT_F saveRect = Rect(left + panelW - 58.0f, top + 74.0f,
            left + panelW - 18.0f, top + 110.0f);
        RegisterInteractive(saveRect);
        const bool inputHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), app.configInputRect);
        const bool saveHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), saveRect);

        FillRound(app.configInputRect, 1.0f,
            app.configNameFocused || inputHover ? MakeColour(0x101318) : MakeColour(0x090B0E));
        DrawRound(app.configInputRect, 1.0f,
            app.configNameFocused ? app.settings.accentColour : MakeColour(0x171A20), 1.0f);
        DrawTextValue(app.configName.empty() ? L"Enter config name" : app.configName,
            Rect(app.configInputRect.left + 10.0f, app.configInputRect.top,
                app.configInputRect.right - 10.0f, app.configInputRect.bottom),
            app.text13.Get(), app.configName.empty() ? MakeColour(0x5E636A) : ui.text);

        if (app.configNameFocused && ((GetTickCount64() / 500ULL) % 2ULL) == 0ULL)
        {
            const float caretX = std::min(app.configInputRect.left + 11.0f +
                static_cast<float>(app.configName.size()) * 7.0f,
                app.configInputRect.right - 12.0f);
            DrawLine(caretX, app.configInputRect.top + 9.0f,
                caretX, app.configInputRect.bottom - 9.0f,
                app.settings.accentColour, 1.0f);
        }

        FillRound(saveRect, 1.0f, saveHover ? MakeColour(0x153C27) : MakeColour(0x102B1D));
        DrawRound(saveRect, 1.0f, MakeColour(0x48D17A), 1.0f);
        DrawConfigIcon(L"\xE74E", saveRect, MakeColour(0x65E592));

        DrawTextValue(L"Saved configs",
            Rect(left + 4.0f, top + 124.0f, left + panelW - 4.0f, top + 153.0f),
            app.text13.Get(), ui.muted);
        DrawLine(left + 4.0f, top + 154.0f, left + panelW - 4.0f, top + 154.0f,
            MakeColour(0x17191D, 0.8f));

        const float listLeft = left + 4.0f;
        const float listRight = left + panelW - 4.0f;
        const float listTop = top + 164.0f;
        const float rowH = 35.0f;
        constexpr int visibleRows = 7;
        const int maxStart = std::max(0, static_cast<int>(app.configs.size()) - visibleRows);
        const int startIndex = std::clamp(static_cast<int>(std::round(app.configScroll)), 0, maxStart);
        const int endIndex = std::min(static_cast<int>(app.configs.size()), startIndex + visibleRows);

        if (app.configs.empty())
        {
            DrawConfigIcon(L"\xE8A5", Rect(listLeft, listTop + 42.0f, listRight, listTop + 82.0f),
                MakeColour(0x5E636A));
            DrawTextValue(L"No saved configs", Rect(listLeft, listTop + 82.0f, listRight, listTop + 112.0f),
                app.text13.Get(), ui.muted, DWRITE_TEXT_ALIGNMENT_CENTER);
        }
        else
        {
            float rowY = listTop;
            for (int index = startIndex; index < endIndex; ++index)
            {
                const D2D1_RECT_F row = Rect(listLeft, rowY, listRight, rowY + rowH - 3.0f);
                const D2D1_RECT_F loadRect = Rect(row.right - 70.0f, row.top + 2.0f,
                    row.right - 38.0f, row.bottom - 2.0f);
                const D2D1_RECT_F deleteRect = Rect(row.right - 34.0f, row.top + 2.0f,
                    row.right - 2.0f, row.bottom - 2.0f);
                RegisterInteractive(row);
                RegisterInteractive(loadRect);
                RegisterInteractive(deleteRect);

                const bool rowHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), row);
                const bool loadHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), loadRect);
                const bool deleteHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), deleteRect);
                const bool selected = index == app.selectedConfig;

                if (selected || rowHover)
                    FillRound(row, 1.0f, selected ? MakeColour(0x161A20, 0.96f)
                        : WithAlpha(ui.hover, 0.48f));

                DrawConfigIcon(L"\xE74E", Rect(row.left + 6.0f, row.top, row.left + 34.0f, row.bottom),
                    selected ? MakeColour(0x65E592) : ui.muted);
                DrawTextValue(app.configs[index], Rect(row.left + 38.0f, row.top,
                    loadRect.left - 6.0f, row.bottom),
                    app.text13.Get(), selected ? ui.text : ui.muted);

                if (loadHover)
                    FillRound(loadRect, 1.0f, MakeColour(0x122A3A));
                if (deleteHover)
                    FillRound(deleteRect, 1.0f, MakeColour(0x3A1518));
                DrawConfigIcon(L"\xE72C", loadRect, loadHover ? MakeColour(0x8DC5FF) : MakeColour(0x6993B5));
                DrawConfigIcon(L"\xE74D", deleteRect, deleteHover ? MakeColour(0xFF6B73) : MakeColour(0xC84B52));

                if (app.clicked && !app.clickConsumed)
                {
                    if (deleteHover)
                    {
                        const std::wstring deleting = app.configs[index];
                        if (DeleteConfig(deleting))
                        {
                            if (app.configName == deleting)
                                app.configName.clear();
                            RefreshConfigs();
                            ShowConfigStatus(L"Config deleted.", true);
                            app.clickConsumed = true;
                            break;
                        }
                        else
                        {
                            ShowConfigStatus(L"Could not delete config.", false);
                        }
                        app.clickConsumed = true;
                    }
                    else if (loadHover)
                    {
                        if (LoadConfig(app.configs[index]))
                        {
                            app.selectedConfig = index;
                            app.configName = app.configs[index];
                            ShowConfigStatus(L"Config loaded.", true);
                        }
                        else
                        {
                            ShowConfigStatus(L"Could not load config.", false);
                        }
                        app.clickConsumed = true;
                    }
                    else if (rowHover)
                    {
                        app.selectedConfig = index;
                        app.configName = app.configs[index];
                        app.configNameFocused = false;
                        app.clickConsumed = true;
                    }
                }
                rowY += rowH;
            }
        }

        if (app.configStatusTimer > 0.0f)
        {
            DrawTextValue(app.configStatus,
                Rect(left + 4.0f, top + panelH - 38.0f,
                    left + panelW - 4.0f, top + panelH - 10.0f),
                app.text12.Get(),
                app.configStatusOk ? MakeColour(0x65E592) : MakeColour(0xFF6B73),
                DWRITE_TEXT_ALIGNMENT_CENTER);
        }

        if (app.clicked && !app.clickConsumed)
        {
            const float mx = static_cast<float>(app.mouse.x);
            const float my = static_cast<float>(app.mouse.y);
            if (closeHover)
            {
                app.configOpen = false;
                app.configNameFocused = false;
                app.clickConsumed = true;
            }
            else if (inputHover)
            {
                app.configNameFocused = true;
                app.clickConsumed = true;
            }
            else if (saveHover)
            {
                app.configNameFocused = false;
                SaveNamedConfig();
                app.clickConsumed = true;
            }
            else if (!Hit(mx, my, app.configPanelRect))
            {
                app.configNameFocused = false;
            }
        }
    }

