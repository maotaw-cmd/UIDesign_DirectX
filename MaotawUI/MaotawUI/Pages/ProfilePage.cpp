#include "../Core/App.h"


void App::DrawProfilePage()
{
    DrawLine(Layout::SplitX + 0.5f, 13.0f, Layout::SplitX + 0.5f, 447.0f, Theme::LineSoft);

    DrawText(L"Profile", Layout::LeftX, 20.0f, Layout::LeftRight, 48.0f,
        Theme::Text, text_);
    DrawText(L"Configuration manager", Layout::RightX, 20.0f,
        Layout::RightRight, 48.0f, Theme::SubText, small_, DWRITE_TEXT_ALIGNMENT_TRAILING);

    Divider(Layout::LeftX, Layout::LeftRight, 61.0f);
    Divider(Layout::RightX, Layout::RightRight, 61.0f);

    TextInput(L"CONFIG NAME", configName_, configNameFocused_,
        Layout::LeftX, Layout::LeftRight, 79.0f);

    if (ActionButton(L"Save config", Glyph::Save,
        Layout::LeftX, 160.0f, Layout::LeftRight, 199.0f, true))
    {
        configNameFocused_ = false;
        const std::wstring cleanName = SanitizeConfigName(configName_);
        if (cleanName.empty())
        {
            StatusMessage(L"Enter a valid config name.", false);
        }
        else if (SaveConfig(cleanName))
        {
            configName_ = cleanName;
            RefreshConfigs();
            const auto iterator = std::find(configs_.begin(), configs_.end(), cleanName);
            if (iterator != configs_.end())
                selectedConfig_ = static_cast<int>(std::distance(configs_.begin(), iterator));
            EnsureSelectedConfigVisible();
            StatusMessage(L"Config saved successfully.", true);
        }
        else
        {
            StatusMessage(L"Config could not be saved.", false);
        }
    }

    if (ActionButton(L"Load selected", Glyph::Load,
        Layout::LeftX, 212.0f, Layout::LeftRight, 251.0f))
    {
        configNameFocused_ = false;
        if (!HasSelectedConfig())
        {
            StatusMessage(L"Select a config first.", false);
        }
        else if (LoadConfig(SelectedConfig()))
        {
            configName_ = SelectedConfig();
            StatusMessage(L"Config loaded successfully.", true);
        }
        else
        {
            StatusMessage(L"Config could not be loaded.", false);
        }
    }

    if (ActionButton(L"Delete selected", Glyph::Delete,
        Layout::LeftX, 264.0f, Layout::LeftRight, 303.0f, false, true))
    {
        configNameFocused_ = false;
        if (!HasSelectedConfig())
        {
            StatusMessage(L"Select a config first.", false);
        }
        else
        {
            const std::wstring deletedName = SelectedConfig();
            if (DeleteConfig(deletedName))
            {
                configName_.clear();
                RefreshConfigs();
                StatusMessage(L"Config deleted.", true);
            }
            else
            {
                StatusMessage(L"Config could not be deleted.", false);
            }
        }
    }

    FillRound(Layout::LeftX, 328.0f, Layout::LeftRight, 388.0f,
        5.0f, WithAlpha(Theme::Panel, 0.82f));
    DrawRound(Layout::LeftX, 328.0f, Layout::LeftRight, 388.0f,
        5.0f, Theme::Line, 1.0f);
    DrawIcon(Glyph::Info, Layout::LeftX + 10.0f, 328.0f,
        Layout::LeftX + 42.0f, 388.0f, Theme::Muted);
    DrawText(L"Configs save every current menu option and both custom colors.",
        Layout::LeftX + 46.0f, 334.0f, Layout::LeftRight - 12.0f, 382.0f,
        Theme::SubText, tiny_);

    DrawText(L"SAVED CONFIGS", Layout::RightX, 76.0f,
        Layout::RightRight, 96.0f, Theme::Muted, tiny_);

    FillRound(Layout::RightX, 102.0f, Layout::RightRight, 382.0f,
        6.0f, WithAlpha(Theme::Panel, 0.88f));
    DrawRound(Layout::RightX, 102.0f, Layout::RightRight, 382.0f,
        6.0f, Theme::Line, 1.0f);

    if (configs_.empty())
    {
        DrawIcon(Glyph::Folder, Layout::RightX, 151.0f,
            Layout::RightRight, 198.0f, Theme::Muted);
        DrawText(L"No saved configs", Layout::RightX + 18.0f, 200.0f,
            Layout::RightRight - 18.0f, 226.0f, Theme::SubText, small_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
        DrawText(L"Enter a name and save your first profile.",
            Layout::RightX + 28.0f, 229.0f, Layout::RightRight - 28.0f, 259.0f,
            Theme::Muted, tiny_, DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else
    {
        constexpr int visibleRows = 7;
        constexpr float listTop = 110.0f;
        constexpr float rowStep = 38.0f;
        constexpr float rowHeight = 35.0f;
        constexpr float listBottom = listTop + rowStep * visibleRows;

        const int totalItems = static_cast<int>(configs_.size());
        const int maximumScroll = std::max(0, totalItems - visibleRows);
        const bool showScrollbar = maximumScroll > 0;

        configScrollIndex_ = std::clamp(
            configScrollIndex_,
            0,
            maximumScroll);

        const float itemRight = showScrollbar
            ? Layout::RightRight - 20.0f
            : Layout::RightRight - 7.0f;

        const int visibleCount = std::min(
            visibleRows,
            totalItems - configScrollIndex_);

        for (int visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex)
        {
            const int configIndex = configScrollIndex_ + visibleIndex;
            const float rowTop = listTop + rowStep * visibleIndex;
            const float rowBottom = rowTop + rowHeight;
            const bool selected = configIndex == selectedConfig_;
            const bool hovered = Hover(
                Layout::RightX + 7.0f,
                rowTop,
                itemRight,
                rowBottom);

            if (selected || hovered)
            {
                FillRound(
                    Layout::RightX + 7.0f,
                    rowTop,
                    itemRight,
                    rowBottom,
                    4.0f,
                    selected
                    ? WithAlpha(settings_.accentColor, 0.14f)
                    : Theme::RowHover);
            }

            if (selected)
            {
                FillRound(
                    Layout::RightX + 8.0f,
                    rowTop + 8.0f,
                    Layout::RightX + 10.5f,
                    rowBottom - 8.0f,
                    1.0f,
                    settings_.accentColor);
            }

            DrawIcon(
                Glyph::Save,
                Layout::RightX + 16.0f,
                rowTop,
                Layout::RightX + 42.0f,
                rowBottom,
                selected
                ? settings_.accentColor
                : hovered
                ? Theme::SubText
                : Theme::Muted);

            DrawText(
                configs_[configIndex],
                Layout::RightX + 48.0f,
                rowTop,
                itemRight - 8.0f,
                rowBottom,
                selected
                ? Theme::Text
                : hovered
                ? Theme::Text
                : Theme::SubText,
                small_);

            if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
            {
                selectedConfig_ = configIndex;
                configName_ = configs_[configIndex];
                configNameFocused_ = false;
                clickConsumed_ = true;
            }
        }

        if (showScrollbar)
        {
            const float trackLeft = Layout::RightRight - 12.0f;
            const float trackRight = Layout::RightRight - 8.0f;
            const float trackTop = listTop;
            const float trackBottom = listBottom - 3.0f;
            const float trackHeight = trackBottom - trackTop;

            const float thumbHeight = std::max(
                34.0f,
                trackHeight *
                static_cast<float>(visibleRows) /
                static_cast<float>(totalItems));

            const float thumbTravel = std::max(
                1.0f,
                trackHeight - thumbHeight);

            float thumbTop = trackTop +
                thumbTravel *
                static_cast<float>(configScrollIndex_) /
                static_cast<float>(maximumScroll);

            bool thumbHovered = Hover(
                trackLeft - 4.0f,
                thumbTop - 2.0f,
                trackRight + 4.0f,
                thumbTop + thumbHeight + 2.0f);

            const bool trackHovered = Hover(
                trackLeft - 5.0f,
                trackTop,
                trackRight + 5.0f,
                trackBottom);

            if (!colorPickerOpen_ && clicked_ && trackHovered)
            {
                if (thumbHovered)
                {
                    configScrollbarGrabOffset_ =
                        static_cast<float>(mouse_.y) - thumbTop;
                }
                else
                {
                    configScrollbarGrabOffset_ = thumbHeight * 0.5f;
                    const float ratio = Clamp(
                        (static_cast<float>(mouse_.y) -
                            trackTop -
                            configScrollbarGrabOffset_) /
                        thumbTravel,
                        0.0f,
                        1.0f);

                    configScrollIndex_ = static_cast<int>(std::round(
                        ratio * static_cast<float>(maximumScroll)));
                }

                configScrollbarDragging_ = true;
                clickConsumed_ = true;
            }

            if (mouseDown_ && configScrollbarDragging_)
            {
                const float ratio = Clamp(
                    (static_cast<float>(mouse_.y) -
                        trackTop -
                        configScrollbarGrabOffset_) /
                    thumbTravel,
                    0.0f,
                    1.0f);

                configScrollIndex_ = static_cast<int>(std::round(
                    ratio * static_cast<float>(maximumScroll)));

                configScrollIndex_ = std::clamp(
                    configScrollIndex_,
                    0,
                    maximumScroll);

                thumbTop = trackTop +
                    thumbTravel *
                    static_cast<float>(configScrollIndex_) /
                    static_cast<float>(maximumScroll);

                thumbHovered = true;
            }

            // Accent-derived track and thumb so the scrollbar follows the
            // currently selected custom accent color.
            FillRound(
                trackLeft,
                trackTop,
                trackRight,
                trackBottom,
                2.0f,
                WithAlpha(settings_.accentColor, 0.12f));

            FillRound(
                trackLeft,
                thumbTop,
                trackRight,
                thumbTop + thumbHeight,
                2.0f,
                WithAlpha(
                    settings_.accentColor,
                    thumbHovered || configScrollbarDragging_
                    ? 1.0f
                    : 0.72f));
        }
    }

    if (!statusText_.empty())
    {
        const D2D1_COLOR_F statusColor = statusSuccess_ ? Theme::Success : Theme::Danger;
        FillRound(Layout::LeftX, 405.0f, Layout::RightRight, 442.0f,
            5.0f, WithAlpha(statusColor, 0.10f));
        DrawRound(Layout::LeftX, 405.0f, Layout::RightRight, 442.0f,
            5.0f, WithAlpha(statusColor, 0.52f), 1.0f);
        DrawIcon(statusSuccess_ ? Glyph::Check : Glyph::Info,
            Layout::LeftX + 8.0f, 405.0f, Layout::LeftX + 38.0f, 442.0f,
            statusColor);
        DrawText(statusText_, Layout::LeftX + 43.0f, 405.0f,
            Layout::RightRight - 12.0f, 442.0f, statusColor, small_);
    }
}
