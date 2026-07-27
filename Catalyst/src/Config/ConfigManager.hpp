#pragma once

// Configuration persistence only.
// This file owns config paths, validation, save/load/delete, and list refresh.
// It intentionally contains no drawing or DirectX rendering code.

    std::filesystem::path ConfigDirectory()
    {
        wchar_t appData[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
        std::filesystem::path directory = length > 0
            ? std::filesystem::path(appData) / L"Catalyst" / L"Configs"
            : std::filesystem::current_path() / L"Configs";

        std::error_code error;
        std::filesystem::create_directories(directory, error);
        return directory;
    }

    std::wstring CleanConfigName(const std::wstring& name)
    {
        std::wstring clean;
        clean.reserve(name.size());
        for (wchar_t character : name)
        {
            if (std::iswalnum(character) || character == L' ' ||
                character == L'-' || character == L'_')
            {
                clean.push_back(character);
            }
        }

        while (!clean.empty() && clean.front() == L' ')
            clean.erase(clean.begin());
        while (!clean.empty() && clean.back() == L' ')
            clean.pop_back();
        if (clean.size() > 36)
            clean.resize(36);
        return clean;
    }

    std::filesystem::path ConfigPath(const std::wstring& name)
    {
        return ConfigDirectory() / (CleanConfigName(name) + L".cfg");
    }

    void WriteColour(std::wofstream& file, const wchar_t* prefix, const D2D1_COLOR_F& colour)
    {
        file << prefix << L"R=" << colour.r << L'\n';
        file << prefix << L"G=" << colour.g << L'\n';
        file << prefix << L"B=" << colour.b << L'\n';
    }

    bool SaveConfig(const std::wstring& name)
    {
        const std::wstring clean = CleanConfigName(name);
        if (clean.empty())
            return false;

        std::wofstream file(ConfigPath(clean), std::ios::trunc);
        if (!file)
            return false;

        auto writeBool = [&](const wchar_t* key, bool value) { file << key << L'=' << (value ? 1 : 0) << L'\n'; };
        auto writeInt = [&](const wchar_t* key, int value) { file << key << L'=' << value << L'\n'; };
        auto writeFloat = [&](const wchar_t* key, float value) { file << key << L'=' << value << L'\n'; };

        file << L"[Catalyst]\n";
        writeBool(L"box", app.settings.box);
        writeInt(L"boxStyle", app.settings.boxStyle);
        writeFloat(L"boxThickness", app.settings.boxThickness);
        writeFloat(L"boxFillAlpha", app.settings.boxFillAlpha);
        writeBool(L"healthBar", app.settings.healthBar);
        writeInt(L"healthBarStyle", app.settings.healthBarStyle);
        writeFloat(L"healthBarThickness", app.settings.healthBarThickness);
        writeFloat(L"healthOutlineThickness", app.settings.healthOutlineThickness);
        writeBool(L"healthText", app.settings.healthText);
        writeBool(L"damageText", app.settings.damageText);
        writeBool(L"name", app.settings.name);
        writeBool(L"distance", app.settings.distance);
        writeBool(L"skeleton", app.settings.skeleton);
        writeFloat(L"skeletonThickness", app.settings.skeletonThickness);
        writeBool(L"snapline", app.settings.snapline);
        writeInt(L"snaplineOrigin", app.settings.snaplineOrigin);
        writeFloat(L"snaplineThickness", app.settings.snaplineThickness);
        writeBool(L"soundWalk", app.settings.soundWalk);
        writeInt(L"soundWalkAnimationStyle", app.settings.soundWalkAnimationStyle);
        writeFloat(L"soundWalkSpeed", app.settings.soundWalkSpeed);
        writeFloat(L"soundWalkExpansion", app.settings.soundWalkExpansion);
        writeFloat(L"soundWalkThickness", app.settings.soundWalkThickness);
        writeBool(L"soundMarker", app.settings.soundMarker);
        writeBool(L"modelOutline", app.settings.visualGlow);
        writeFloat(L"modelOutlineThickness", app.settings.visualGlowThickness);
        writeBool(L"rotateModel", app.settings.rotateModel);
        writeFloat(L"modelScale", app.settings.modelScale);
        writeBool(L"particlesEnabled", app.particlesEnabled);
        writeFloat(L"particleSpeed", app.particleSpeed);
        writeFloat(L"particleAmount", app.particleAmount);

        WriteColour(file, L"box", app.settings.boxColour);
        WriteColour(file, L"boxFill", app.settings.boxFillColour);
        WriteColour(file, L"health", app.settings.healthColour);
        WriteColour(file, L"healthBackground", app.settings.healthBackgroundColour);
        WriteColour(file, L"healthOutline", app.settings.healthOutlineColour);
        WriteColour(file, L"damageText", app.settings.damageTextColour);
        WriteColour(file, L"skeleton", app.settings.skeletonColour);
        WriteColour(file, L"snapline", app.settings.snaplineColour);
        WriteColour(file, L"soundWalk", app.settings.soundWalkColour);
        WriteColour(file, L"soundMarker", app.settings.soundMarkerColour);
        WriteColour(file, L"modelOutline", app.settings.visualGlowColour);
        WriteColour(file, L"model", app.settings.modelColour);
        WriteColour(file, L"accent", app.settings.accentColour);

        file.flush();
        return file.good();
    }

    bool LoadConfig(const std::wstring& name)
    {
        std::wifstream file(ConfigPath(name));
        if (!file)
            return false;

        std::wstring line;
        while (std::getline(file, line))
        {
            if (line.empty() || line.front() == L'[')
                continue;
            const std::size_t separator = line.find(L'=');
            if (separator == std::wstring::npos)
                continue;

            const std::wstring key = line.substr(0, separator);
            const std::wstring value = line.substr(separator + 1);
            try
            {
                const int intValue = std::stoi(value);
                const bool boolValue = intValue != 0;
                const float floatValue = std::stof(value);

                if (key == L"box") app.settings.box = boolValue;
                else if (key == L"boxStyle") app.settings.boxStyle = intValue;
                else if (key == L"boxThickness") app.settings.boxThickness = floatValue;
                else if (key == L"boxFillAlpha") app.settings.boxFillAlpha = floatValue;
                else if (key == L"healthBar") app.settings.healthBar = boolValue;
                else if (key == L"healthBarStyle") app.settings.healthBarStyle = intValue;
                else if (key == L"healthBarThickness") app.settings.healthBarThickness = floatValue;
                else if (key == L"healthOutlineThickness") app.settings.healthOutlineThickness = floatValue;
                else if (key == L"healthText") app.settings.healthText = boolValue;
                else if (key == L"damageText") app.settings.damageText = boolValue;
                else if (key == L"name") app.settings.name = boolValue;
                else if (key == L"distance") app.settings.distance = boolValue;
                else if (key == L"skeleton") app.settings.skeleton = boolValue;
                else if (key == L"skeletonThickness") app.settings.skeletonThickness = floatValue;
                else if (key == L"snapline") app.settings.snapline = boolValue;
                else if (key == L"snaplineOrigin") app.settings.snaplineOrigin = intValue;
                else if (key == L"snaplineThickness") app.settings.snaplineThickness = floatValue;
                else if (key == L"soundWalk") app.settings.soundWalk = boolValue;
                else if (key == L"soundWalkAnimationStyle") app.settings.soundWalkAnimationStyle = intValue;
                else if (key == L"soundWalkSpeed") app.settings.soundWalkSpeed = floatValue;
                else if (key == L"soundWalkExpansion") app.settings.soundWalkExpansion = floatValue;
                else if (key == L"soundWalkThickness") app.settings.soundWalkThickness = floatValue;
                else if (key == L"soundMarker") app.settings.soundMarker = boolValue;
                else if (key == L"modelOutline") app.settings.visualGlow = boolValue;
                else if (key == L"modelOutlineThickness") app.settings.visualGlowThickness = floatValue;
                else if (key == L"rotateModel") app.settings.rotateModel = boolValue;
                else if (key == L"modelScale") app.settings.modelScale = floatValue;
                else if (key == L"particlesEnabled") app.particlesEnabled = boolValue;
                else if (key == L"particleSpeed") app.particleSpeed = floatValue;
                else if (key == L"particleAmount") app.particleAmount = floatValue;

                else if (key == L"boxR") app.settings.boxColour.r = floatValue;
                else if (key == L"boxG") app.settings.boxColour.g = floatValue;
                else if (key == L"boxB") app.settings.boxColour.b = floatValue;
                else if (key == L"boxFillR") app.settings.boxFillColour.r = floatValue;
                else if (key == L"boxFillG") app.settings.boxFillColour.g = floatValue;
                else if (key == L"boxFillB") app.settings.boxFillColour.b = floatValue;
                else if (key == L"healthR") app.settings.healthColour.r = floatValue;
                else if (key == L"healthG") app.settings.healthColour.g = floatValue;
                else if (key == L"healthB") app.settings.healthColour.b = floatValue;
                else if (key == L"healthBackgroundR") app.settings.healthBackgroundColour.r = floatValue;
                else if (key == L"healthBackgroundG") app.settings.healthBackgroundColour.g = floatValue;
                else if (key == L"healthBackgroundB") app.settings.healthBackgroundColour.b = floatValue;
                else if (key == L"healthOutlineR") app.settings.healthOutlineColour.r = floatValue;
                else if (key == L"healthOutlineG") app.settings.healthOutlineColour.g = floatValue;
                else if (key == L"healthOutlineB") app.settings.healthOutlineColour.b = floatValue;
                else if (key == L"damageTextR") app.settings.damageTextColour.r = floatValue;
                else if (key == L"damageTextG") app.settings.damageTextColour.g = floatValue;
                else if (key == L"damageTextB") app.settings.damageTextColour.b = floatValue;
                else if (key == L"skeletonR") app.settings.skeletonColour.r = floatValue;
                else if (key == L"skeletonG") app.settings.skeletonColour.g = floatValue;
                else if (key == L"skeletonB") app.settings.skeletonColour.b = floatValue;
                else if (key == L"snaplineR") app.settings.snaplineColour.r = floatValue;
                else if (key == L"snaplineG") app.settings.snaplineColour.g = floatValue;
                else if (key == L"snaplineB") app.settings.snaplineColour.b = floatValue;
                else if (key == L"soundWalkR") app.settings.soundWalkColour.r = floatValue;
                else if (key == L"soundWalkG") app.settings.soundWalkColour.g = floatValue;
                else if (key == L"soundWalkB") app.settings.soundWalkColour.b = floatValue;
                else if (key == L"soundMarkerR") app.settings.soundMarkerColour.r = floatValue;
                else if (key == L"soundMarkerG") app.settings.soundMarkerColour.g = floatValue;
                else if (key == L"soundMarkerB") app.settings.soundMarkerColour.b = floatValue;
                else if (key == L"modelOutlineR") app.settings.visualGlowColour.r = floatValue;
                else if (key == L"modelOutlineG") app.settings.visualGlowColour.g = floatValue;
                else if (key == L"modelOutlineB") app.settings.visualGlowColour.b = floatValue;
                else if (key == L"modelR") app.settings.modelColour.r = floatValue;
                else if (key == L"modelG") app.settings.modelColour.g = floatValue;
                else if (key == L"modelB") app.settings.modelColour.b = floatValue;
                else if (key == L"accentR") app.settings.accentColour.r = floatValue;
                else if (key == L"accentG") app.settings.accentColour.g = floatValue;
                else if (key == L"accentB") app.settings.accentColour.b = floatValue;
            }
            catch (...)
            {
            }
        }

        app.settings.boxStyle = std::clamp(app.settings.boxStyle, 0, 4);
        app.settings.healthBarStyle = std::clamp(app.settings.healthBarStyle, 0, 5);
        app.settings.snaplineOrigin = std::clamp(app.settings.snaplineOrigin, 0, 2);
        app.settings.soundWalkAnimationStyle = std::clamp(app.settings.soundWalkAnimationStyle, 0, 3);
        app.settings.boxThickness = Clamp(app.settings.boxThickness, 0.6f, 6.0f);
        app.settings.boxFillAlpha = Clamp(app.settings.boxFillAlpha, 0.0f, 1.0f);
        app.settings.healthBarThickness = Clamp(app.settings.healthBarThickness, 2.0f, 16.0f);
        app.settings.healthOutlineThickness = Clamp(app.settings.healthOutlineThickness, 0.0f, 4.0f);
        app.settings.snaplineThickness = Clamp(app.settings.snaplineThickness, 0.6f, 6.0f);
        app.settings.skeletonThickness = Clamp(app.settings.skeletonThickness, 0.6f, 5.0f);
        app.settings.visualGlowThickness = Clamp(app.settings.visualGlowThickness, 0.5f, 6.0f);
        app.settings.soundWalkSpeed = Clamp(app.settings.soundWalkSpeed, 0.35f, 1.80f);
        app.settings.soundWalkExpansion = Clamp(app.settings.soundWalkExpansion, 0.70f, 2.30f);
        app.settings.soundWalkThickness = Clamp(app.settings.soundWalkThickness, 0.60f, 4.00f);
        app.settings.modelScale = Clamp(app.settings.modelScale, 0.55f, 1.05f);
        app.particleSpeed = Clamp(app.particleSpeed, 4.0f, 60.0f);
        app.particleAmount = Clamp(app.particleAmount, 8.0f, 70.0f);

        D2D1_COLOR_F* colours[] = {
            &app.settings.boxColour, &app.settings.boxFillColour,
            &app.settings.healthColour, &app.settings.healthBackgroundColour,
            &app.settings.healthOutlineColour, &app.settings.damageTextColour,
            &app.settings.skeletonColour,
            &app.settings.snaplineColour, &app.settings.soundWalkColour,
            &app.settings.soundMarkerColour, &app.settings.visualGlowColour,
            &app.settings.modelColour, &app.settings.accentColour
        };
        for (D2D1_COLOR_F* colour : colours)
        {
            colour->r = Clamp(colour->r, 0.0f, 1.0f);
            colour->g = Clamp(colour->g, 0.0f, 1.0f);
            colour->b = Clamp(colour->b, 0.0f, 1.0f);
            colour->a = 1.0f;
        }
        return true;
    }

    bool DeleteConfig(const std::wstring& name)
    {
        std::error_code error;
        return std::filesystem::remove(ConfigPath(name), error);
    }

    void RefreshConfigs()
    {
        const std::wstring previous =
            app.selectedConfig >= 0 && app.selectedConfig < static_cast<int>(app.configs.size())
            ? app.configs[app.selectedConfig]
            : L"";

        app.configs.clear();
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(ConfigDirectory(), error))
        {
            if (error)
                break;
            if (entry.is_regular_file() && entry.path().extension() == L".cfg")
                app.configs.push_back(entry.path().stem().wstring());
        }
        // Always provide one usable starter config on first launch.
        if (app.configs.empty())
        {
            constexpr const wchar_t* defaultConfigName = L"default_confing";
            if (SaveConfig(defaultConfigName))
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
            const auto iterator = std::find(app.configs.begin(), app.configs.end(), L"default_confing");
            if (iterator != app.configs.end())
                app.selectedConfig = static_cast<int>(std::distance(app.configs.begin(), iterator));
        }
        const float maxScroll = std::max(0.0f, static_cast<float>(app.configs.size()) - 7.0f);
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
        const std::wstring clean = CleanConfigName(app.configName);
        if (clean.empty())
        {
            ShowConfigStatus(L"Enter a valid config name.", false);
            return;
        }
        if (!SaveConfig(clean))
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

