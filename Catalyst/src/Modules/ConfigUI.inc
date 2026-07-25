


// Configuration screen drawing and config-list actions. File persistence stays in Config.cpp.
// Included once through ProjectAll.h. Do not add this file to Source Files.

// Config list and UI state helpers. Persistence is implemented in Config.cpp.

    void RefreshConfigs()
    {
        const std::wstring previous =
            app.selectedConfig >= 0 && app.selectedConfig < static_cast<int>(app.configs.size())
            ? app.configs[app.selectedConfig]
            : L"";

        app.configs.clear();
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(Config::Directory(), error))
        {
            if (error)
                break;
            if (entry.is_regular_file() && entry.path().extension() == L".cfg")
                app.configs.push_back(entry.path().stem().wstring());
        }
        // Always provide one usable starter config on first launch.
        if (app.configs.empty())
        {
            constexpr const wchar_t* defaultConfigName = L"default_config";
            if (Config::Save(defaultConfigName, app.settings, { app.particlesEnabled, app.particleSpeed, app.particleAmount }))
            {
                app.configs.emplace_back(defaultConfigName);
                app.configName = defaultConfigName;
            }
        }

        std::sort(app.configs.begin(), app.configs.end());
        app.selectedConfig = -1;
        if (!previous.empty())
        {
            const auto iterator = std::find(app.configs.begin(), app.configs.end(), previous);
            if (iterator != app.configs.end())
                app.selectedConfig = static_cast<int>(std::distance(app.configs.begin(), iterator));
        }
        else
        {
            const auto iterator = std::find(app.configs.begin(), app.configs.end(), L"default_config");
            if (iterator != app.configs.end())
                app.selectedConfig = static_cast<int>(std::distance(app.configs.begin(), iterator));
        }
        const float maxScroll = (std::max)(0.0f, static_cast<float>(app.configs.size()) - 7.0f);
        app.configScroll = Clamp(app.configScroll, 0.0f, maxScroll);
    }

    void ShowConfigStatus(const std::wstring& text, bool ok)
    {
        app.configStatus = text;
        app.configStatusOk = ok;
        app.configStatusTimer = 2.5f;
    }

    void SaveNamedConfig()
    {
        const std::wstring clean = Config::CleanName(app.configName);
        if (clean.empty())
        {
            ShowConfigStatus(L"Enter a valid config name.", false);
            return;
        }
        if (!Config::Save(clean, app.settings, { app.particlesEnabled, app.particleSpeed, app.particleAmount }))
        {
            ShowConfigStatus(L"Could not save config.", false);
            return;
        }

        app.configName = clean;
        RefreshConfigs();
        const auto iterator = std::find(app.configs.begin(), app.configs.end(), clean);
        if (iterator != app.configs.end())
            app.selectedConfig = static_cast<int>(std::distance(app.configs.begin(), iterator));
        ShowConfigStatus(L"Config saved.", true);
    }


// ===== BEGIN DIRECTX RENDERER =====

    // ------------------------------------------------------------------------
    // Device creation
    // ------------------------------------------------------------------------


    void DrawConfigIcon(const wchar_t* icon, const D2D1_RECT_F& rect, const D2D1_COLOR_F& colour)
    {
        DrawTextValue(icon, rect, app.iconFont.Get(), colour, DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    void DrawConfigPanel()
    {
        // Compact configuration popup opened from the top save icon.
        const float cardW = 390.0f;
        const float cardH = 410.0f;
        if (!app.configPanelPositioned)
        {
            app.configPanelX = (static_cast<float>(app.width) - cardW) * 0.5f;
            app.configPanelY = TOP_H + 18.0f;
            app.configPanelPositioned = true;
        }

        // Keep the draggable card inside the client area.
        app.configPanelX = Clamp(app.configPanelX, 4.0f, (std::max)(4.0f, static_cast<float>(app.width) - cardW - 4.0f));
        app.configPanelY = Clamp(app.configPanelY, TOP_H + 4.0f, (std::max)(TOP_H + 4.0f, static_cast<float>(app.height) - cardH - 4.0f));
        const float left = app.configPanelX;
        const float top = app.configPanelY;
        const D2D1_RECT_F card = Rect(left, top, left + cardW, top + cardH);
        app.configPanelRect = card;

        // Nearly opaque popup card with a sharp 1 px radius.
        FillRound(card, 1.0f, MakeColour(0x0B0D11, 0.98f));
        DrawRound(card, 1.0f, MakeColour(0x232731, 0.92f), 1.0f);

        DrawTextValue(L"Configurations",
            Rect(left + 18.0f, top + 12.0f, left + 280.0f, top + 44.0f),
            app.text19Bold.Get(), ui.white);

        const D2D1_RECT_F closeRect = Rect(card.right - 42.0f, top + 8.0f,
            card.right - 10.0f, top + 40.0f);
        RegisterInteractive(closeRect);
        const bool closeHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), closeRect);
        if (closeHover) FillRound(closeRect, 1.0f, WithAlpha(ui.hover, 0.65f));
        DrawConfigIcon(L"\xE711", closeRect, closeHover ? ui.white : ui.muted);

        DrawTextValue(L"Name", Rect(left + 18.0f, top + 50.0f, left + 100.0f, top + 72.0f),
            app.text12.Get(), ui.muted);

        app.configInputRect = Rect(left + 18.0f, top + 74.0f, card.right - 64.0f, top + 108.0f);
        const D2D1_RECT_F saveRect = Rect(card.right - 54.0f, top + 74.0f, card.right - 18.0f, top + 108.0f);
        RegisterInteractive(app.configInputRect);
        RegisterInteractive(saveRect);
        const bool inputHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), app.configInputRect);
        const bool saveHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), saveRect);

        FillRound(app.configInputRect, 1.0f, app.configNameFocused || inputHover ? MakeColour(0x12151B) : MakeColour(0x090B0E));
        DrawRound(app.configInputRect, 1.0f, app.configNameFocused ? app.settings.accentColour : MakeColour(0x242832), 1.0f);
        DrawTextValue(app.configName.empty() ? L"Enter config name" : app.configName,
            Rect(app.configInputRect.left + 10.0f, app.configInputRect.top,
                app.configInputRect.right - 10.0f, app.configInputRect.bottom),
            app.text13.Get(), app.configName.empty() ? MakeColour(0x5E636A) : ui.text);

        FillRound(saveRect, 1.0f, saveHover ? WithAlpha(app.settings.accentColour, 0.28f) : MakeColour(0x171A20));
        DrawConfigIcon(L"\xE74E", saveRect, saveHover ? ui.white : app.settings.accentColour);

        DrawTextValue(L"Saved configs", Rect(left + 18.0f, top + 122.0f, card.right - 18.0f, top + 148.0f),
            app.text13.Get(), ui.muted);
        DrawLine(left + 18.0f, top + 150.0f, card.right - 18.0f, top + 150.0f, MakeColour(0x242832, 0.85f));

        const float listLeft = left + 18.0f;
        const float listRight = card.right - 18.0f;
        const float listTop = top + 160.0f;
        const float rowH = 34.0f;
        constexpr int visibleRows = 6;
        const int maxStart = (std::max)(0, static_cast<int>(app.configs.size()) - visibleRows);
        const int startIndex = std::clamp(static_cast<int>(std::round(app.configScroll)), 0, maxStart);
        const int endIndex = (std::min)(static_cast<int>(app.configs.size()), startIndex + visibleRows);

        if (app.configs.empty())
        {
            DrawTextValue(L"No saved configs", Rect(listLeft, listTop + 55.0f, listRight, listTop + 90.0f),
                app.text13.Get(), ui.muted, DWRITE_TEXT_ALIGNMENT_CENTER);
        }
        else
        {
            float rowY = listTop;
            for (int index = startIndex; index < endIndex; ++index)
            {
                const D2D1_RECT_F row = Rect(listLeft, rowY, listRight, rowY + rowH - 3.0f);
                const D2D1_RECT_F loadRect = Rect(row.right - 66.0f, row.top + 2.0f, row.right - 36.0f, row.bottom - 2.0f);
                const D2D1_RECT_F deleteRect = Rect(row.right - 32.0f, row.top + 2.0f, row.right - 2.0f, row.bottom - 2.0f);
                RegisterInteractive(row); RegisterInteractive(loadRect); RegisterInteractive(deleteRect);
                const bool rowHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), row);
                const bool loadHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), loadRect);
                const bool deleteHover = Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), deleteRect);
                const bool selected = index == app.selectedConfig;
                if (selected || rowHover) FillRound(row, 1.0f, selected ? MakeColour(0x171B22, 0.98f) : WithAlpha(ui.hover, 0.48f));
                DrawTextValue(app.configs[index], Rect(row.left + 10.0f, row.top, loadRect.left - 6.0f, row.bottom),
                    app.text13.Get(), selected ? ui.text : ui.muted);
                DrawConfigIcon(L"\xE72C", loadRect, loadHover ? ui.white : MakeColour(0x6993B5));
                DrawConfigIcon(L"\xE74D", deleteRect, deleteHover ? MakeColour(0xFF6B73) : MakeColour(0xC84B52));

                if (app.clicked && !app.clickConsumed)
                {
                    if (deleteHover)
                    {
                        const std::wstring deleting = app.configs[index];
                        if (Config::Delete(deleting)) { RefreshConfigs(); ShowConfigStatus(L"Config deleted.", true); }
                        else ShowConfigStatus(L"Could not delete config.", false);
                        app.clickConsumed = true;
                    }
                    else if (loadHover)
                    {
                        if (Config::Load(app.configs[index], app.settings, app.particlesEnabled, app.particleSpeed, app.particleAmount))
                        { app.selectedConfig = index; app.configName = app.configs[index]; ShowConfigStatus(L"Config loaded.", true); }
                        else ShowConfigStatus(L"Could not load config.", false);
                        app.clickConsumed = true;
                    }
                    else if (rowHover)
                    {
                        app.selectedConfig = index; app.configName = app.configs[index]; app.configNameFocused = false; app.clickConsumed = true;
                    }
                }
                rowY += rowH;
            }
        }

        if (app.configStatusTimer > 0.0f)
            DrawTextValue(app.configStatus, Rect(left + 18.0f, card.bottom - 35.0f, card.right - 18.0f, card.bottom - 10.0f),
                app.text12.Get(), app.configStatusOk ? MakeColour(0x65E592) : MakeColour(0xFF6B73), DWRITE_TEXT_ALIGNMENT_CENTER);

        if (app.clicked && !app.clickConsumed)
        {
            const float mx = static_cast<float>(app.mouse.x), my = static_cast<float>(app.mouse.y);
            if (closeHover || !Hit(mx, my, card))
            { app.configOpen = false; app.configNameFocused = false; app.clickConsumed = true; }
            else if (inputHover)
            { app.configNameFocused = true; app.clickConsumed = true; }
            else if (saveHover)
            { app.configNameFocused = false; SaveNamedConfig(); app.clickConsumed = true; }
        }
    }


