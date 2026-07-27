// Included inside class App. Edit this file to customise this page.

    std::filesystem::path ConfigDirectory() const {
        wchar_t modulePath[MAX_PATH]{};
        GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        return std::filesystem::path(modulePath).parent_path() / L"configs";
    }
    std::wstring SafeConfigName(std::wstring name) const {
        std::wstring clean;
        for (wchar_t ch : name) {
            if (iswalnum(ch) || ch == L'_' || ch == L'-')
                clean.push_back(ch);
        }
        if (clean.empty()) clean = L"default_config";
        return clean;
    }
    void RefreshConfigList() {
        configFiles.clear();
        std::error_code ec;
        const auto dir = ConfigDirectory();
        std::filesystem::create_directories(dir, ec);
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() == L".cfg")
                configFiles.push_back(entry.path().stem().wstring());
        }
        std::sort(configFiles.begin(), configFiles.end());
        selectedConfig = -1;
        for (int i = 0; i < static_cast<int>(configFiles.size()); ++i) {
            if (configFiles[i] == configName) {
                selectedConfig = i;
                break;
            }
        }

        const int maximumScroll = std::max(0, static_cast<int>(configFiles.size()) - 5);
        configListScroll = std::clamp(configListScroll, 0, maximumScroll);
        if (selectedConfig >= 0) {
            if (selectedConfig < configListScroll)
                configListScroll = selectedConfig;
            else if (selectedConfig >= configListScroll + 5)
                configListScroll = selectedConfig - 4;
            configListScroll = std::clamp(configListScroll, 0, maximumScroll);
        }
    }
    bool SaveNamedConfig() {
        configName = SafeConfigName(configName);
        std::error_code ec;
        std::filesystem::create_directories(ConfigDirectory(), ec);
        std::wofstream file(ConfigDirectory() / (configName + L".cfg"), std::ios::trunc);
        if (!file) return false;

        file << L"ARCANE_CONFIG 1\n";
        for (const auto& value : checks) file << L"B " << value.first << L" " << (value.second ? 1 : 0) << L"\n";
        for (const auto& value : sliders) file << L"S " << value.first << L" " << std::setprecision(9) << value.second << L"\n";
        for (const auto& value : choices) file << L"I " << value.first << L" " << value.second << L"\n";
        for (const auto& value : colors)
            file << L"C " << value.first << L" " << value.second.r << L" " << value.second.g << L" " << value.second.b << L"\n";
        for (const auto& value : hotkeys) file << L"K " << value.first << L" " << value.second << L"\n";

        RefreshConfigList();
        return true;
    }
    bool LoadNamedConfig() {
        configName = SafeConfigName(configName);
        std::wifstream file(ConfigDirectory() / (configName + L".cfg"));
        if (!file) return false;

        std::wstring line;
        std::getline(file, line);
        while (std::getline(file, line)) {
            std::wistringstream stream(line);
            wchar_t type = 0;
            int id = 0;
            stream >> type >> id;
            if (!stream) continue;
            if (type == L'B') {
                int value = 0; stream >> value; checks[id] = value != 0;
            } else if (type == L'S') {
                float value = 0.0f; stream >> value; sliders[id] = std::clamp(value, 0.0f, 1.0f);
            } else if (type == L'I') {
                int value = 0; stream >> value; choices[id] = value;
            } else if (type == L'C') {
                RGB value{}; stream >> value.r >> value.g >> value.b; colors[id] = value;
            } else if (type == L'K') {
                std::wstring value; std::getline(stream >> std::ws, value); hotkeys[id] = value;
            }
        }
        RefreshConfigList();
        InvalidateRect(hwnd, nullptr, FALSE);
        return true;
    }
    bool DeleteNamedConfig() {
        configName = SafeConfigName(configName);
        std::error_code ec;
        const bool removed = std::filesystem::remove(ConfigDirectory() / (configName + L".cfg"), ec);
        RefreshConfigList();
        if (!configFiles.empty()) {
            selectedConfig = std::clamp(selectedConfig, 0, static_cast<int>(configFiles.size()) - 1);
            configName = configFiles[static_cast<std::size_t>(selectedConfig)];
        }
        return removed && !ec;
    }
    void DrawConfigInput(float x, float y, float w) {
        const bool active = configInputActive;
        Round(D2D1::RectF(x, y, x + w, y + 18), 3, active ? 0x2B3033 : 0x22272A);
        RoundStroke(D2D1::RectF(x, y, x + w, y + 18), 3, active ? 0x697277 : 0x343A3D);
        Text(configName, x + 6, y, w - 12, 18, 0xD7DADB, f8.Get());
        HitBox(x, y, x + w, y + 18, 7600);
    }
    void DrawConfigSaveIcon(float x, float y, unsigned colour) {
        // Compact floppy/save icon, drawn as vectors.
        RoundStroke(D2D1::RectF(x, y, x + 11, y + 11), 1.5f, colour, 1.0f);
        Fill(D2D1::RectF(x + 2.0f, y + 1.5f, x + 8.0f, y + 4.4f), colour);
        RoundStroke(D2D1::RectF(x + 2.2f, y + 6.2f, x + 8.8f, y + 9.6f),
                    0.8f, colour, 0.9f);
    }
    void DrawConfigList(float x, float y, float w, float h) {
        constexpr int visible = 5;
        constexpr float rowHeight = 22.0f;
        constexpr float rowVisualHeight = 19.0f;

        const int maximumScroll =
            std::max(0, static_cast<int>(configFiles.size()) - visible);
        configListScroll = std::clamp(configListScroll, 0, maximumScroll);

        for (int row = 0; row < visible; ++row) {
            const int index = configListScroll + row;
            if (index >= static_cast<int>(configFiles.size())) break;

            const float top = y + row * rowHeight;
            const bool selected = index == selectedConfig;

            if (selected)
                Round(D2D1::RectF(x, top, x + w - 7.0f, top + rowVisualHeight),
                      2.0f, 0x292F32);

            const unsigned iconColour = selected ? 0xF0F2F3 : 0x8D9599;
            DrawConfigSaveIcon(x + 5.0f, top + 4.0f, iconColour);

            Text(configFiles[static_cast<std::size_t>(index)],
                 x + 23.0f, top, w - 34.0f, rowVisualHeight,
                 selected ? 0xF0F2F3 : 0xAAB0B3, f8.Get());

            HitBox(x, top, x + w - 7.0f, top + rowVisualHeight, 7700 + index);
        }

        if (maximumScroll > 0) {
            const float trackX = x + w - 3.0f;
            const float trackTop = y;
            const float trackBottom = y + visible * rowHeight - 3.0f;
            const float trackHeight = trackBottom - trackTop;
            const float thumbHeight = std::max(
                24.0f,
                trackHeight * (static_cast<float>(visible) /
                               static_cast<float>(configFiles.size())));
            const float available = trackHeight - thumbHeight;
            const float ratio = static_cast<float>(configListScroll) /
                                static_cast<float>(maximumScroll);
            const float thumbTop = trackTop + available * ratio;

            // Very thin scrollbar.
            Fill(D2D1::RectF(trackX, trackTop, trackX + 1.25f, trackBottom),
                 0x343A3D, 0.45f);
            Round(D2D1::RectF(trackX, thumbTop,
                             trackX + 1.25f, thumbTop + thumbHeight),
                  0.6f, 0xF2F4F5, 0.92f);

            HitBox(trackX - 4.0f, trackTop, trackX + 5.0f, trackBottom, 7890);
        }
    }
    void UpdateConfigScrollbarFromMouse(float mouseY) {
        constexpr int visible = 5;
        constexpr float listY = 160.0f;
        constexpr float rowHeight = 22.0f;
        const int maximumScroll =
            std::max(0, static_cast<int>(configFiles.size()) - visible);
        if (maximumScroll <= 0) return;

        const float trackTop = listY;
        const float trackBottom = listY + visible * rowHeight - 3.0f;
        const float trackHeight = trackBottom - trackTop;
        const float thumbHeight = std::max(
            24.0f,
            trackHeight * (static_cast<float>(visible) /
                           static_cast<float>(configFiles.size())));
        const float available = std::max(1.0f, trackHeight - thumbHeight);
        const float thumbTop = std::clamp(
            mouseY - configScrollbarDragOffset,
            trackTop, trackTop + available);

        configListScroll = std::clamp(
            static_cast<int>(std::lround(
                ((thumbTop - trackTop) / available) * maximumScroll)),
            0, maximumScroll);
        InvalidateRect(hwnd, nullptr, FALSE);
    }
    void DrawSettings() {
        Title(L"Settings");

        Card(L"Configurations", 153, 34, 194, 321);
        Label(L"Config name", 160, 62);
        DrawConfigInput(160, 80, 176);

        Button(L"Save", 160, 108, 52, 19, 900);
        Button(L"Load", 222, 108, 52, 19, 901);
        Button(L"Delete", 284, 108, 52, 19, 903);

        Label(L"Saved configs", 160, 140, 176);
        DrawConfigList(160, 160, 176, 110);

        Card(L"Interface", 357, 34, 197, 164);
        Label(L"Accent colour", 364, 64); ColorBox(525, 66, 405);

        Label(L"Window transparency", 364, 91);
        Text(std::to_wstring((int)(sliders[230] * 100)) + L"%", 510, 91, 35, 14,
             0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);
        Slider(364, 112, 180, 230);

        RowCheck(L"Animations", 364, 138, 43);
        RowCheck(L"Tooltips", 364, 162, 44);
    }
