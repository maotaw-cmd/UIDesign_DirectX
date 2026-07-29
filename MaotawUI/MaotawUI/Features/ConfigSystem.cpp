#include "../Core/App.h"

namespace
{
    void ValidateVisualProfile(VisualProfile& profile)
    {
        profile.boxStyle = std::clamp(profile.boxStyle, 0, 2);
        profile.healthPosition = std::clamp(profile.healthPosition, 0, 3);
        profile.armorPosition = std::clamp(profile.armorPosition, 0, 3);
        profile.healthStyle = std::clamp(profile.healthStyle, 0, 2);
        profile.armorStyle = std::clamp(profile.armorStyle, 0, 2);
        profile.snaplinePosition = std::clamp(profile.snaplinePosition, 0, 2);
        profile.namePosition = std::clamp(profile.namePosition, 0, 1);

        profile.boxThickness = Clamp(profile.boxThickness, 0.5f, 5.0f);
        profile.fillOpacity = Clamp(profile.fillOpacity, 0.0f, 100.0f);
        profile.skeletonThickness = Clamp(profile.skeletonThickness, 0.5f, 5.0f);
        profile.glowStrength = Clamp(profile.glowStrength, 0.0f, 100.0f);
        profile.arrowSize = Clamp(profile.arrowSize, 4.0f, 40.0f);
        profile.maxDistance = Clamp(profile.maxDistance, 1.0f, 1000.0f);
    }
}


std::filesystem::path App::ConfigDirectory() const
{
    wchar_t appData[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);

    std::filesystem::path directory = length > 0
        ? std::filesystem::path(appData) / L"MaotawUI" / L"Configs"
        : std::filesystem::current_path() / L"Configs";

    std::error_code error;
    std::filesystem::create_directories(directory, error);
    return directory;
}

std::wstring App::SanitizeConfigName(const std::wstring& name) const
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

    if (clean.size() > 32)
        clean.resize(32);

    return clean;
}

std::filesystem::path App::ConfigPath(const std::wstring& name) const
{
    return ConfigDirectory() / (SanitizeConfigName(name) + L".cfg");
}

bool App::SaveConfig(const std::wstring& name)
{
    const std::wstring cleanName = SanitizeConfigName(name);
    if (cleanName.empty()) return false;
    std::ofstream file(ConfigPath(cleanName), std::ios::binary | std::ios::trunc);
    if (!file) return false;
    const std::uint32_t magic = 0x4D534346;
    const std::uint32_t version = 8;
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&settings_), sizeof(settings_));
    file.flush();
    return file.good();
}

bool App::LoadConfig(const std::wstring& name)
{
    const std::wstring cleanName = SanitizeConfigName(name);
    if (cleanName.empty()) return false;
    std::ifstream file(ConfigPath(cleanName), std::ios::binary);
    if (!file) return false;
    std::uint32_t magic = 0, version = 0;
    Settings loaded{};
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&loaded), sizeof(loaded));
    if (!file || magic != 0x4D534346 || version != 8) return false;
    loaded.dpiScale = Clamp(loaded.dpiScale, 80.0f, 200.0f);
    loaded.particleSpeed = Clamp(loaded.particleSpeed, 0.0f, 60.0f);
    loaded.particleCount = Clamp(loaded.particleCount, 8.0f, 48.0f);
    loaded.connectionDistance = Clamp(loaded.connectionDistance, 40.0f, 180.0f);
    loaded.particleOpacity = Clamp(loaded.particleOpacity, 5.0f, 100.0f);
    loaded.windowOpacity = Clamp(loaded.windowOpacity, 25.0f, 100.0f);
    loaded.windowBorderOpacity = Clamp(loaded.windowBorderOpacity, 0.0f, 100.0f);
    ValidateVisualProfile(loaded.local);
    ValidateVisualProfile(loaded.enemy);
    settings_ = loaded;
    return true;
}

bool App::DeleteConfig(const std::wstring& name)
{
    const std::wstring cleanName = SanitizeConfigName(name);
    if (cleanName.empty())
        return false;

    std::error_code error;
    return std::filesystem::remove(ConfigPath(cleanName), error) && !error;
}

void App::RefreshConfigs()
{
    const std::wstring previous = SelectedConfig();
    configs_.clear();

    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(ConfigDirectory(), error))
    {
        if (error)
            break;

        if (entry.is_regular_file() && entry.path().extension() == L".cfg")
            configs_.push_back(entry.path().stem().wstring());
    }

    std::sort(configs_.begin(), configs_.end());
    selectedConfig_ = -1;

    if (!previous.empty())
    {
        const auto iterator = std::find(configs_.begin(), configs_.end(), previous);
        if (iterator != configs_.end())
            selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), iterator));
    }

    if (selectedConfig_ < 0 && !configs_.empty())
        selectedConfig_ = 0;

    EnsureSelectedConfigVisible();
}

void App::EnsureSelectedConfigVisible()
{
    constexpr int visibleRows = 7;
    const int maximumScroll = std::max(
        0,
        static_cast<int>(configs_.size()) - visibleRows);

    configScrollIndex_ = std::clamp(
        configScrollIndex_,
        0,
        maximumScroll);

    if (!HasSelectedConfig())
        return;

    if (selectedConfig_ < configScrollIndex_)
    {
        configScrollIndex_ = selectedConfig_;
    }
    else if (selectedConfig_ >= configScrollIndex_ + visibleRows)
    {
        configScrollIndex_ = selectedConfig_ - visibleRows + 1;
    }

    configScrollIndex_ = std::clamp(
        configScrollIndex_,
        0,
        maximumScroll);
}

bool App::HasSelectedConfig() const
{
    return selectedConfig_ >= 0 &&
        selectedConfig_ < static_cast<int>(configs_.size());
}

std::wstring App::SelectedConfig() const
{
    return HasSelectedConfig() ? configs_[selectedConfig_] : L"";
}

void App::DrawConfigPopup()
{
    if (!configPopupOpen_)
        return;

    FillRect(0.0f, 0.0f, static_cast<float>(Layout::Width),
        static_cast<float>(Layout::Height), MakeColor(0x000000, 0.56f));

    // Smaller draggable card. It uses the exact same base colour as the window.
    constexpr float cardW = 350.0f;
    constexpr float cardH = 365.0f;
    configPopupX_ = Clamp(configPopupX_, 8.0f,
        static_cast<float>(Layout::Width) - cardW - 8.0f);
    configPopupY_ = Clamp(configPopupY_, 8.0f,
        static_cast<float>(Layout::Height) - cardH - 8.0f);

    if (configPopupDragging_ && mouseDown_)
    {
        configPopupX_ = Clamp(static_cast<float>(mouse_.x) - configPopupDragOffsetX_,
            8.0f, static_cast<float>(Layout::Width) - cardW - 8.0f);
        configPopupY_ = Clamp(static_cast<float>(mouse_.y) - configPopupDragOffsetY_,
            8.0f, static_cast<float>(Layout::Height) - cardH - 8.0f);
    }
    else if (!mouseDown_)
    {
        configPopupDragging_ = false;
    }

    const float x = configPopupX_;
    const float y = configPopupY_;
    const float right = x + cardW;
    const float bottom = y + cardH;

    FillRound(x, y, right, bottom, 4.0f, Theme::Window);
    DrawRound(x, y, right, bottom, 4.0f, Theme::Line, 1.0f);

    const float headerBottom = y + 43.0f;
    const float closeL = right - 39.0f;
    const float closeT = y + 7.0f;
    const float closeR = right - 9.0f;
    const float closeB = y + 37.0f;
    const bool closeHover = Hover(closeL, closeT, closeR, closeB);
    const bool headerHover = Hover(x, y, right, headerBottom) && !closeHover;

    if (headerHover)
        FillRound(x + 1.0f, y + 1.0f, right - 1.0f, headerBottom, 4.0f,
            WithAlpha(Theme::RowHover, 0.28f));

    DrawText(L"Configurations", x + 16.0f, y + 7.0f,
        right - 48.0f, headerBottom, Theme::White, text_);
    DrawLine(x + 12.0f, headerBottom, right - 12.0f, headerBottom,
        Theme::LineSoft, 1.0f);

    if (closeHover)
        FillRound(closeL, closeT, closeR, closeB, 3.0f,
            WithAlpha(Theme::RowHover, 0.65f));
    DrawIcon(Glyph::Close, closeL, closeT, closeR, closeB,
        closeHover ? Theme::White : Theme::Muted);

    if (clicked_ && !clickConsumed_ && headerHover)
    {
        configPopupDragging_ = true;
        configPopupDragOffsetX_ = static_cast<float>(mouse_.x) - x;
        configPopupDragOffsetY_ = static_cast<float>(mouse_.y) - y;
        configNameFocused_ = false;
        clickConsumed_ = true;
    }

    DrawText(L"Name", x + 16.0f, y + 49.0f, x + 90.0f, y + 67.0f,
        Theme::Muted, tiny_);

    const float inputL = x + 16.0f;
    const float inputT = y + 69.0f;
    const float inputR = right - 58.0f;
    const float inputB = y + 101.0f;
    const float saveL = right - 50.0f;
    const float saveT = inputT;
    const float saveR = right - 16.0f;
    const float saveB = inputB;

    const bool inputHover = Hover(inputL, inputT, inputR, inputB);
    const bool saveHover = Hover(saveL, saveT, saveR, saveB);

    FillRound(inputL, inputT, inputR, inputB, 3.0f,
        configNameFocused_ || inputHover ? Theme::RowHover : Theme::Row);
    DrawRound(inputL, inputT, inputR, inputB, 3.0f,
        configNameFocused_ ? settings_.accentColor : Theme::Line, 1.0f);
    DrawText(configName_.empty() ? L"Enter config name" : configName_,
        inputL + 9.0f, inputT, inputR - 9.0f, inputB,
        configName_.empty() ? Theme::Muted : Theme::Text, small_);

    if (configNameFocused_ && (GetTickCount64() / 500ULL) % 2ULL == 0ULL)
    {
        const float estimatedWidth = std::min(
            static_cast<float>(configName_.size()) * 5.6f,
            (inputR - inputL) - 23.0f);
        DrawLine(inputL + 9.0f + estimatedWidth, inputT + 7.0f,
            inputL + 9.0f + estimatedWidth, inputB - 7.0f,
            settings_.accentColor, 1.0f);
    }

    FillRound(saveL, saveT, saveR, saveB, 3.0f,
        saveHover ? WithAlpha(settings_.accentColor, 0.28f) : Theme::Row);
    DrawRound(saveL, saveT, saveR, saveB, 3.0f,
        saveHover ? settings_.accentColor : Theme::Line, 1.0f);
    DrawIcon(Glyph::Save, saveL, saveT, saveR, saveB,
        saveHover ? Theme::White : settings_.accentColor);

    DrawText(L"Saved configs", x + 16.0f, y + 111.0f,
        right - 16.0f, y + 134.0f, Theme::Muted, small_);
    DrawLine(x + 16.0f, y + 136.0f, right - 16.0f, y + 136.0f,
        Theme::LineSoft, 1.0f);

    const float listLeft = x + 16.0f;
    const float listRight = right - 16.0f;
    const float listTop = y + 145.0f;
    constexpr float rowH = 32.0f;
    constexpr int visibleRows = 5;
    const int maxStart = std::max(0, static_cast<int>(configs_.size()) - visibleRows);
    configScrollIndex_ = std::clamp(configScrollIndex_, 0, maxStart);
    const int endIndex = std::min(static_cast<int>(configs_.size()),
        configScrollIndex_ + visibleRows);

    if (configs_.empty())
    {
        DrawText(L"No saved configs", listLeft, listTop + 48.0f,
            listRight, listTop + 82.0f, Theme::Muted, small_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else
    {
        float rowY = listTop;
        for (int index = configScrollIndex_; index < endIndex; ++index)
        {
            const float rowTop = rowY;
            const float rowBottom = rowY + rowH - 3.0f;
            const float loadL = listRight - 62.0f;
            const float loadR = listRight - 34.0f;
            const float deleteL = listRight - 30.0f;
            const float deleteR = listRight - 2.0f;

            const bool rowHover = Hover(listLeft, rowTop, listRight, rowBottom);
            const bool loadHover = Hover(loadL, rowTop + 2.0f, loadR, rowBottom - 2.0f);
            const bool deleteHover = Hover(deleteL, rowTop + 2.0f, deleteR, rowBottom - 2.0f);
            const bool selected = index == selectedConfig_;

            if (selected || rowHover)
                FillRound(listLeft, rowTop, listRight, rowBottom, 3.0f,
                    selected ? Theme::RowHover : WithAlpha(Theme::RowHover, 0.52f));

            DrawIcon(Glyph::Save, listLeft + 3.0f, rowTop,
                listLeft + 27.0f, rowBottom,
                selected ? settings_.accentColor : Theme::Muted);
            DrawText(configs_[index], listLeft + 29.0f, rowTop,
                loadL - 5.0f, rowBottom,
                selected || rowHover ? Theme::Text : Theme::Muted, small_);
            DrawIcon(Glyph::Load, loadL, rowTop + 2.0f, loadR, rowBottom - 2.0f,
                loadHover ? Theme::White : Theme::Blue);
            DrawIcon(Glyph::Delete, deleteL, rowTop + 2.0f, deleteR, rowBottom - 2.0f,
                deleteHover ? Theme::Danger : WithAlpha(Theme::Danger, 0.72f));

            if (clicked_ && !clickConsumed_)
            {
                if (deleteHover)
                {
                    const std::wstring deleting = configs_[index];
                    configNameFocused_ = false;
                    if (DeleteConfig(deleting))
                    {
                        RefreshConfigs();
                        StatusMessage(L"Config deleted.", true);
                    }
                    else
                        StatusMessage(L"Could not delete config.", false);
                    clickConsumed_ = true;
                }
                else if (loadHover)
                {
                    configNameFocused_ = false;
                    const std::wstring loading = configs_[index];
                    if (LoadConfig(loading))
                    {
                        selectedConfig_ = index;
                        configName_ = loading;
                        StatusMessage(L"Config loaded.", true);
                    }
                    else
                        StatusMessage(L"Could not load config.", false);
                    clickConsumed_ = true;
                }
                else if (rowHover)
                {
                    selectedConfig_ = index;
                    configName_ = configs_[index];
                    configNameFocused_ = false;
                    clickConsumed_ = true;
                }
            }
            rowY += rowH;
        }
    }

    if (statusTimer_ > 0.0f && !statusText_.empty())
    {
        const D2D1_COLOR_F c = statusSuccess_ ? Theme::Success : Theme::Danger;
        DrawText(statusText_, x + 16.0f, bottom - 31.0f,
            right - 16.0f, bottom - 8.0f, c, tiny_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    if (clicked_ && !clickConsumed_)
    {
        const float mx = static_cast<float>(mouse_.x);
        const float my = static_cast<float>(mouse_.y);

        if (closeHover || !Hit(mx, my, x, y, right, bottom))
        {
            configPopupOpen_ = false;
            configPopupDragging_ = false;
            configNameFocused_ = false;
            clickConsumed_ = true;
        }
        else if (inputHover)
        {
            configNameFocused_ = true;
            SetFocus(hwnd_);
            clickConsumed_ = true;
        }
        else if (saveHover)
        {
            configNameFocused_ = false;
            const std::wstring cleanName = SanitizeConfigName(configName_);
            if (cleanName.empty())
                StatusMessage(L"Enter a valid config name.", false);
            else if (SaveConfig(cleanName))
            {
                configName_ = cleanName;
                RefreshConfigs();
                const auto it = std::find(configs_.begin(), configs_.end(), cleanName);
                if (it != configs_.end())
                    selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), it));
                EnsureSelectedConfigVisible();
                StatusMessage(L"Config saved.", true);
            }
            else
                StatusMessage(L"Could not save config.", false);
            clickConsumed_ = true;
        }
        else
        {
            configNameFocused_ = false;
            clickConsumed_ = true;
        }
    }
}
