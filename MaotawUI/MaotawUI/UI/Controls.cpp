#include "../Core/App.h"


void App::SidebarButton(int index, const wchar_t* glyph, float y)
{
    const float left = 14.0f;
    const float right = 58.0f;
    const float top = y;
    const float bottom = y + 43.0f;
    const bool hovered = Hover(left, top, right, bottom);
    const bool selected = index == selectedPage_;

    if (hovered)
        FillRound(left, top, right, bottom, 5.0f, Theme::RowHover);

    if (selected)
    {
        FillRound(13.0f, y + 14.0f, 15.0f, y + 29.0f, 1.0f, WithAlpha(settings_.accentColor, 0.90f));
    }

    DrawIcon(
        glyph,
        left,
        top,
        right,
        bottom,
        selected ? Theme::Text : hovered ? Theme::SubText : Theme::Muted);

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        selectedPage_ = index;

        // A sidebar page change owns the click and closes every floating UI.
        // This also recovers from a stale Local/Enemy selector state.
        openCombo_ = 0;
        targetPopupOpen_ = false;
        visualSettingsPopup_ = 0;
        configPopupOpen_ = false;
        configNameFocused_ = false;
        activeSlider_ = ActiveSlider::None;
        clickConsumed_ = true;
    }
}

void App::ToggleRow(const std::wstring& label, bool& value, float left, float right, float y, bool dividerBelow)
{
    const float height = 39.0f;
    const bool hovered = Hover(left, y, right, y + height);

    if (hovered)
        FillRect(left, y, right, y + height, WithAlpha(Theme::RowHover, 0.42f));

    DrawText(label, left, y, right - 40.0f, y + height, hovered ? Theme::Text : Theme::SubText, small_);

    const float centerX = right - 9.0f;
    const float centerY = y + height * 0.5f;

    SetBrush(value ? settings_.accentColor : Theme::Muted);
    renderTarget_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), 5.2f, 5.2f), brush_, 1.1f);

    if (value)
    {
        SetBrush(settings_.accentColor);
        renderTarget_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), 2.3f, 2.3f), brush_);
    }

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        value = !value;
        clickConsumed_ = true;
    }

    if (dividerBelow)
        Divider(left, right, y + height);
}

std::wstring App::FormatNumber(float value, int decimals)
{
    wchar_t buffer[64]{};
    if (decimals <= 0)
        swprintf_s(buffer, L"%.0f", value);
    else
        swprintf_s(buffer, L"%.*f", decimals, value);
    return buffer;
}

void App::SliderControl(const std::wstring& label, float& value, float minimum, float maximum,
    float left, float right, float y, ActiveSlider slider, int decimals)
{
    DrawText(label, left, y, right - 58.0f, y + 21.0f, Theme::SubText, small_);
    DrawText(FormatNumber(value, decimals), right - 56.0f, y, right, y + 21.0f,
        Theme::Text, small_, DWRITE_TEXT_ALIGNMENT_TRAILING);

    const float trackLeft = left;
    const float trackRight = right - 16.0f;
    const float trackY = y + 34.0f;
    const float ratio = Clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
    const float thumbX = trackLeft + (trackRight - trackLeft) * ratio;

    FillRound(trackLeft, trackY - 2.0f, trackRight, trackY + 2.0f, 2.0f, Theme::Track);
    FillRound(trackLeft, trackY - 2.0f, thumbX, trackY + 2.0f, 2.0f, settings_.accentColor);
    FillRound(thumbX - 5.3f, trackY - 5.3f, thumbX + 5.3f, trackY + 5.3f, 2.2f, Theme::White);

    const bool hovered = Hover(trackLeft - 5.0f, trackY - 11.0f, trackRight + 5.0f, trackY + 11.0f);
    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        activeSlider_ = slider;
        clickConsumed_ = true;
    }

    if (!colorPickerOpen_ && openCombo_ == 0 && mouseDown_ && activeSlider_ == slider)
    {
        const float mouseRatio = Clamp((static_cast<float>(mouse_.x) - trackLeft) / (trackRight - trackLeft), 0.0f, 1.0f);
        value = minimum + mouseRatio * (maximum - minimum);
    }
}

void App::ComboControl(int id, const std::wstring& label, const std::vector<std::wstring>& items,
    int& selected, float left, float right, float y)
{
    selected = std::clamp(selected, 0, static_cast<int>(items.size()) - 1);

    // Compact label and compact dropdown field.
    DrawText(label, left, y, right, y + 15.0f, Theme::Muted, tiny_);

    const float boxTop = y + 17.0f;
    const float boxBottom = boxTop + 27.0f;
    const bool hovered = Hover(left, boxTop, right, boxBottom);
    const bool opened = openCombo_ == id;

    FillRound(left, boxTop, right, boxBottom, 4.0f, Theme::Row);
    DrawRound(left, boxTop, right, boxBottom, 4.0f,
        opened ? settings_.accentColor : hovered ? WithAlpha(Theme::White, 0.13f) : Theme::Line, 1.0f);

    DrawText(items[selected], left + 10.0f, boxTop, right - 28.0f, boxBottom,
        Theme::Text, tiny_);
    DrawIcon(Glyph::Chevron, right - 25.0f, boxTop, right - 5.0f, boxBottom,
        opened ? settings_.accentColor : Theme::Muted);

    if (!colorPickerOpen_ && hovered && clicked_ && (openCombo_ == 0 || opened))
    {
        openCombo_ = opened ? 0 : id;
        clickConsumed_ = true;
    }

    if (openCombo_ == id)
    {
        comboPopup_.id = id;
        comboPopup_.left = left;
        comboPopup_.itemHeight = 23.0f;
        const float popupHeight = comboPopup_.itemHeight * static_cast<float>(items.size());
        comboPopup_.top = boxBottom + 3.0f;
        if (comboPopup_.top + popupHeight > static_cast<float>(Layout::Height) - 8.0f)
            comboPopup_.top = boxTop - popupHeight - 3.0f;
        comboPopup_.right = right;
        comboPopup_.items = items;
        comboPopup_.selected = &selected;
    }
}

void App::ColorRow(const std::wstring& label, D2D1_COLOR_F& color,
    float left, float right, float y)
{
    const float height = 41.0f;
    const bool hovered = Hover(left, y, right, y + height);

    if (hovered && !colorPickerOpen_)
        FillRect(left, y, right, y + height, WithAlpha(Theme::RowHover, 0.42f));

    DrawText(label, left, y, right - 45.0f, y + height,
        hovered && !colorPickerOpen_ ? Theme::Text : Theme::SubText, small_);

    const float swatchLeft = right - 22.0f;
    const float swatchTop = y + 15.0f;
    FillRound(swatchLeft, swatchTop, right - 3.0f, swatchTop + 10.0f, 2.0f, color);
    DrawRound(swatchLeft, swatchTop, right - 3.0f, swatchTop + 10.0f,
        2.0f, WithAlpha(Theme::White, 0.10f));

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        OpenColorPicker(color);
        clickConsumed_ = true;
    }
}

bool App::ActionButton(const std::wstring& label, const wchar_t* glyph,
    float left, float top, float right, float bottom, bool accent, bool danger)
{
    const bool hovered = Hover(left, top, right, bottom);

    D2D1_COLOR_F background = Theme::Row;
    D2D1_COLOR_F border = Theme::Line;
    D2D1_COLOR_F foreground = Theme::Text;

    if (accent)
    {
        background = hovered
            ? WithAlpha(settings_.accentColor, 0.90f)
            : WithAlpha(settings_.accentColor, 0.72f);
        border = settings_.accentColor;
        foreground = Theme::White;
    }
    else if (danger)
    {
        background = hovered ? MakeColor(0x35151B) : MakeColor(0x241116);
        border = WithAlpha(Theme::Danger, hovered ? 0.95f : 0.55f);
        foreground = hovered ? Theme::White : Theme::Danger;
    }
    else if (hovered)
    {
        background = Theme::RowHover;
        border = WithAlpha(Theme::White, 0.12f);
    }

    FillRound(left, top, right, bottom, 4.0f, background);
    DrawRound(left, top, right, bottom, 4.0f, border, 1.0f);

    if (glyph)
        DrawIcon(glyph, left + 8.0f, top, left + 32.0f, bottom,
            accent ? Theme::White : danger ? Theme::Danger : hovered ? Theme::Text : Theme::Muted);

    DrawText(label,
        left + (glyph ? 35.0f : 8.0f), top, right - 8.0f, bottom,
        foreground, small_, DWRITE_TEXT_ALIGNMENT_CENTER);

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        clickConsumed_ = true;
        return true;
    }

    return false;
}

void App::TextInput(const std::wstring& label, std::wstring& value, bool& focused,
    float left, float right, float y)
{
    DrawText(label, left, y, right, y + 18.0f, Theme::SubText, small_);

    const float top = y + 23.0f;
    const float bottom = top + 38.0f;
    const bool hovered = Hover(left, top, right, bottom);

    FillRound(left, top, right, bottom, 4.0f,
        hovered || focused ? Theme::RowHover : Theme::Row);
    DrawRound(left, top, right, bottom, 4.0f,
        focused ? settings_.accentColor : hovered ? WithAlpha(Theme::White, 0.14f) : Theme::Line,
        1.0f);

    DrawText(value.empty() ? L"Enter config name" : value,
        left + 12.0f, top, right - 12.0f, bottom,
        value.empty() ? Theme::Muted : Theme::Text, small_);

    if (focused && (GetTickCount64() / 500ULL) % 2ULL == 0ULL)
    {
        const float estimatedWidth = std::min(
            static_cast<float>(value.size()) * 5.6f,
            (right - left) - 31.0f);
        DrawLine(left + 12.0f + estimatedWidth, top + 10.0f,
            left + 12.0f + estimatedWidth, bottom - 10.0f,
            settings_.accentColor, 1.0f);
    }

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        focused = true;
        openCombo_ = 0;
        clickConsumed_ = true;
        SetFocus(hwnd_);
    }
}

void App::StatusMessage(const std::wstring& text, bool success)
{
    statusText_ = text;
    statusSuccess_ = success;
    statusTimer_ = 3.2f;
}

void App::DrawComboPopup()
{
    if (colorPickerOpen_ || openCombo_ == 0 || comboPopup_.id != openCombo_ || !comboPopup_.selected)
        return;

    const float popupBottom = comboPopup_.top + comboPopup_.itemHeight * static_cast<float>(comboPopup_.items.size());

    FillRound(comboPopup_.left, comboPopup_.top, comboPopup_.right, popupBottom, 4.0f, Theme::Popup);
    DrawRound(comboPopup_.left, comboPopup_.top, comboPopup_.right, popupBottom, 4.0f, Theme::Line);

    for (std::size_t index = 0; index < comboPopup_.items.size(); ++index)
    {
        const float itemTop = comboPopup_.top + comboPopup_.itemHeight * static_cast<float>(index);
        const float itemBottom = itemTop + comboPopup_.itemHeight;
        const bool hovered = Hover(comboPopup_.left, itemTop, comboPopup_.right, itemBottom);
        const bool selected = static_cast<int>(index) == *comboPopup_.selected;

        // No large hover block; only a subtle outline keeps the list clean.
        if (hovered)
            DrawRound(comboPopup_.left + 3.0f, itemTop + 2.0f,
                comboPopup_.right - 3.0f, itemBottom - 2.0f, 3.0f,
                WithAlpha(Theme::White, 0.10f), 1.0f);

        if (selected)
            FillRound(comboPopup_.left + 5.0f, itemTop + 7.0f,
                comboPopup_.left + 7.0f, itemBottom - 7.0f, 1.0f, settings_.accentColor);

        DrawText(comboPopup_.items[index], comboPopup_.left + 11.0f, itemTop,
            comboPopup_.right - 8.0f, itemBottom,
            selected ? settings_.accentColor : hovered ? Theme::Text : Theme::SubText,
            tiny_);

        if (hovered && clicked_)
        {
            *comboPopup_.selected = static_cast<int>(index);
            openCombo_ = 0;
            clickConsumed_ = true;
        }
    }

    // The popup is modal: clicking its empty padding/card area must never
    // pass through to controls rendered behind it.
    if (clicked_ && Hover(comboPopup_.left, comboPopup_.top, comboPopup_.right, popupBottom))
        clickConsumed_ = true;
}

void App::OpenColorPicker(D2D1_COLOR_F& color)
{
    pickerTarget_ = &color;
    pickerOriginal_ = color;
    pickerWorking_ = color;
    ColorToHsv(color, pickerHue_, pickerSaturation_, pickerValue_);
    colorPickerOpen_ = true;
    activeColorArea_ = -1;
    activeSlider_ = ActiveSlider::None;
    openCombo_ = 0;
}

void App::UpdatePickerColorFromHsv()
{
    pickerWorking_ = HsvToColor(pickerHue_, pickerSaturation_, pickerValue_);
}

void App::DrawColorPicker()
{
    if (!colorPickerOpen_ || !pickerTarget_)
        return;

    FillRect(0.0f, 0.0f, static_cast<float>(Layout::Width),
        static_cast<float>(Layout::Height), MakeColor(0x000000, 0.72f));

    const float boxWidth = 350.0f;
    const float boxHeight = 292.0f;
    const float x = (static_cast<float>(Layout::Width) - boxWidth) * 0.5f;
    const float y = (static_cast<float>(Layout::Height) - boxHeight) * 0.5f;

    FillRect(x, y, x + boxWidth, y + boxHeight, MakeColor(0x0D0E12));
    DrawRect(x, y, x + boxWidth, y + boxHeight, settings_.accentColor, 1.0f);

    DrawText(L"Color picker", x + 15.0f, y + 3.0f,
        x + boxWidth - 15.0f, y + 34.0f, Theme::Text, text_);

    DrawLine(x + 14.0f, y + 37.0f, x + boxWidth - 14.0f, y + 37.0f,
        Theme::Line, 1.0f);

    const float paletteX = x + 18.0f;
    const float paletteY = y + 53.0f;
    const float paletteWidth = 265.0f;
    const float paletteHeight = 174.0f;
    const float hueX = paletteX + paletteWidth + 10.0f;
    const float hueWidth = 18.0f;

    constexpr int columns = 64;
    constexpr int rows = 42;
    const float cellWidth = paletteWidth / static_cast<float>(columns);
    const float cellHeight = paletteHeight / static_cast<float>(rows);

    for (int row = 0; row < rows; ++row)
    {
        const float value = 1.0f - static_cast<float>(row) / static_cast<float>(rows - 1);

        for (int column = 0; column < columns; ++column)
        {
            const float saturation = static_cast<float>(column) / static_cast<float>(columns - 1);
            const float left = paletteX + cellWidth * static_cast<float>(column);
            const float top = paletteY + cellHeight * static_cast<float>(row);

            FillRect(left, top, left + cellWidth + 0.8f, top + cellHeight + 0.8f,
                HsvToColor(pickerHue_, saturation, value));
        }
    }

    constexpr int hueSteps = 90;
    const float hueStepHeight = paletteHeight / static_cast<float>(hueSteps);

    for (int step = 0; step < hueSteps; ++step)
    {
        const float hue = static_cast<float>(step) / static_cast<float>(hueSteps - 1);
        const float top = paletteY + hueStepHeight * static_cast<float>(step);

        FillRect(hueX, top, hueX + hueWidth, top + hueStepHeight + 0.8f,
            HsvToColor(hue, 1.0f, 1.0f));
    }

    const bool paletteHover = Hover(
        paletteX, paletteY, paletteX + paletteWidth, paletteY + paletteHeight);
    const bool hueHover = Hover(
        hueX, paletteY, hueX + hueWidth, paletteY + paletteHeight);

    if (clicked_ && paletteHover)
    {
        activeColorArea_ = 0;
        clickConsumed_ = true;
    }
    else if (clicked_ && hueHover)
    {
        activeColorArea_ = 1;
        clickConsumed_ = true;
    }

    if (mouseDown_ && activeColorArea_ == 0)
    {
        pickerSaturation_ = Clamp(
            (static_cast<float>(mouse_.x) - paletteX) / paletteWidth,
            0.0f,
            1.0f);

        pickerValue_ = 1.0f - Clamp(
            (static_cast<float>(mouse_.y) - paletteY) / paletteHeight,
            0.0f,
            1.0f);

        UpdatePickerColorFromHsv();
    }
    else if (mouseDown_ && activeColorArea_ == 1)
    {
        pickerHue_ = Clamp(
            (static_cast<float>(mouse_.y) - paletteY) / paletteHeight,
            0.0f,
            1.0f);

        UpdatePickerColorFromHsv();
    }

    const float pointX = paletteX + pickerSaturation_ * paletteWidth;
    const float pointY = paletteY + (1.0f - pickerValue_) * paletteHeight;

    SetBrush(MakeColor(0x000000));
    renderTarget_->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(pointX, pointY), 5.2f, 5.2f),
        brush_,
        3.0f);

    SetBrush(Theme::White);
    renderTarget_->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(pointX, pointY), 5.2f, 5.2f),
        brush_,
        1.4f);

    const float hueMarkerY = paletteY + pickerHue_ * paletteHeight;

    FillRect(hueX - 3.0f, hueMarkerY - 2.0f,
        hueX + hueWidth + 3.0f, hueMarkerY + 2.0f, Theme::White);

    DrawRect(hueX - 4.0f, hueMarkerY - 3.0f,
        hueX + hueWidth + 4.0f, hueMarkerY + 3.0f,
        MakeColor(0x000000), 1.0f);

    FillRect(x + 18.0f, y + 239.0f, x + 43.0f, y + 258.0f, pickerWorking_);
    DrawRect(x + 18.0f, y + 239.0f, x + 43.0f, y + 258.0f,
        WithAlpha(Theme::White, 0.12f), 1.0f);

    wchar_t hexText[16]{};
    const int red = std::clamp(
        static_cast<int>(std::round(pickerWorking_.r * 255.0f)), 0, 255);
    const int green = std::clamp(
        static_cast<int>(std::round(pickerWorking_.g * 255.0f)), 0, 255);
    const int blue = std::clamp(
        static_cast<int>(std::round(pickerWorking_.b * 255.0f)), 0, 255);

    swprintf_s(hexText, L"#%02X%02X%02X", red, green, blue);

    DrawText(hexText, x + 51.0f, y + 232.0f,
        x + 163.0f, y + 263.0f, Theme::Text, small_);

    const float buttonY = y + boxHeight - 36.0f;
    const float cancelX = x + boxWidth - 150.0f;
    const float applyX = x + boxWidth - 76.0f;

    const bool cancelHover = Hover(
        cancelX, buttonY, cancelX + 64.0f, buttonY + 24.0f);
    const bool applyHover = Hover(
        applyX, buttonY, applyX + 58.0f, buttonY + 24.0f);

    FillRect(cancelX, buttonY, cancelX + 64.0f, buttonY + 24.0f,
        cancelHover ? Theme::RowHover : Theme::Row);
    DrawRect(cancelX, buttonY, cancelX + 64.0f, buttonY + 24.0f,
        Theme::Line, 1.0f);
    DrawText(L"Cancel", cancelX, buttonY,
        cancelX + 64.0f, buttonY + 24.0f,
        Theme::Text, small_, DWRITE_TEXT_ALIGNMENT_CENTER);

    FillRect(applyX, buttonY, applyX + 58.0f, buttonY + 24.0f,
        applyHover ? WithAlpha(settings_.accentColor, 0.82f) : settings_.accentColor);
    DrawText(L"Apply", applyX, buttonY,
        applyX + 58.0f, buttonY + 24.0f,
        Theme::White, small_, DWRITE_TEXT_ALIGNMENT_CENTER);

    if (clicked_ && cancelHover)
    {
        *pickerTarget_ = pickerOriginal_;
        colorPickerOpen_ = false;
        pickerTarget_ = nullptr;
        activeColorArea_ = -1;
        clickConsumed_ = true;
    }
    else if (clicked_ && applyHover)
    {
        *pickerTarget_ = pickerWorking_;
        colorPickerOpen_ = false;
        pickerTarget_ = nullptr;
        activeColorArea_ = -1;
        clickConsumed_ = true;
    }
}
