#pragma once
#include "Types.h"

class App
{
public:
    bool Initialize(HINSTANCE instance);
    int Run();
    void Shutdown();

    // Stable extension points for custom application logic.
    HWND WindowHandle() const noexcept { return hwnd_; }
    Settings& MutableSettings() noexcept { return settings_; }
    const Settings& CurrentSettings() const noexcept { return settings_; }
    void RequestRedraw() const noexcept { if (hwnd_) InvalidateRect(hwnd_, nullptr, FALSE); }


private:
    enum class ActiveSlider
    {
        None,
        DpiScale,
        ParticleSpeed,
        ParticleCount,
        ConnectionDistance,
        ParticleOpacity,
        WindowOpacity,
        WindowBorderOpacity,
        BoxThickness,
        FillOpacity,
        SkeletonThickness,
        GlowStrength,
        ArrowSize,
        MaxDistance
    };


    struct ComboPopup
    {
        int id = 0;
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float itemHeight = 0.0f;
        std::vector<std::wstring> items;
        int* selected = nullptr;
    };

    HWND hwnd_ = nullptr;

    ID2D1Factory* d2dFactory_ = nullptr;
    ID2D1HwndRenderTarget* renderTarget_ = nullptr;
    ID2D1SolidColorBrush* brush_ = nullptr;
    IDWriteFactory* writeFactory_ = nullptr;
    IDWriteTextFormat* text_ = nullptr;
    IDWriteTextFormat* small_ = nullptr;
    IDWriteTextFormat* tiny_ = nullptr;
    IDWriteTextFormat* damageText_ = nullptr;
    IDWriteTextFormat* icon_ = nullptr;

    POINT mouse_{ -1000, -1000 };
    bool mouseDown_ = false;
    bool clicked_ = false;
    bool clickConsumed_ = false;
    bool windowMoveActive_ = false;
    bool configPopupOpen_ = false;
    float configPopupX_ = 205.0f;
    float configPopupY_ = 52.0f;
    bool configPopupDragging_ = false;
    float configPopupDragOffsetX_ = 0.0f;
    float configPopupDragOffsetY_ = 0.0f;

    int selectedPage_ = 0;
    int selectedVisualTarget_ = 1; // 0 = local, 1 = enemy
    bool targetPopupOpen_ = false;
    int visualSettingsPopup_ = 0;
    float visualPopupX_ = 374.0f;
    float visualPopupY_ = 150.0f;
    bool visualPopupDragging_ = false;
    float visualPopupDragOffsetX_ = 0.0f;
    float visualPopupDragOffsetY_ = 0.0f;
    int openCombo_ = 0;
    ActiveSlider activeSlider_ = ActiveSlider::None;

    std::wstring configName_;
    bool configNameFocused_ = false;
    std::vector<std::wstring> configs_;
    int selectedConfig_ = -1;
    int configScrollIndex_ = 0;
    bool configScrollbarDragging_ = false;
    float configScrollbarGrabOffset_ = 0.0f;
    std::wstring statusText_;
    bool statusSuccess_ = true;
    float statusTimer_ = 0.0f;

    bool colorPickerOpen_ = false;
    D2D1_COLOR_F pickerOriginal_{};
    D2D1_COLOR_F pickerWorking_{};
    D2D1_COLOR_F* pickerTarget_ = nullptr;
    int activeColorArea_ = -1;
    float pickerHue_ = 0.0f;
    float pickerSaturation_ = 0.0f;
    float pickerValue_ = 1.0f;

    Settings settings_{};
    ComboPopup comboPopup_{};

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT CreateFactories();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    void Render();
    void DrawShell(float width, float height);
    void DrawSidebar(float height);
    void DrawMainPage();
    void DrawVisualTargetSelector();
    void DrawVisualTargetPopup();
    void DrawVisualOptionRow(int id, const std::wstring& label, bool& value, float y);
    void DrawVisualSettingsPopup();
    VisualProfile& ActiveVisualProfile();
    void DrawSettingsPage();
    void DrawProfilePage();
    void DrawConfigPopup();
    void DrawOtherPage(int page);
    void DrawConnectedParticles(float width, float height);
    void Draw3DCharacterPreview();

    void SetBrush(const D2D1_COLOR_F& color);
    void FillRect(float left, float top, float right, float bottom, const D2D1_COLOR_F& color);
    void DrawRect(float left, float top, float right, float bottom, const D2D1_COLOR_F& color, float stroke = 1.0f);
    void FillRound(float left, float top, float right, float bottom, float radius, const D2D1_COLOR_F& color);
    void DrawRound(float left, float top, float right, float bottom, float radius, const D2D1_COLOR_F& color, float stroke = 1.0f);
    void DrawLine(float x1, float y1, float x2, float y2, const D2D1_COLOR_F& color, float stroke = 1.0f);
    void DrawText(const std::wstring& value, float left, float top, float right, float bottom,
        const D2D1_COLOR_F& color, IDWriteTextFormat* format = nullptr,
        DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING);
    void DrawIcon(const wchar_t* glyph, float left, float top, float right, float bottom,
        const D2D1_COLOR_F& color);

    bool Hover(float left, float top, float right, float bottom) const;
    void Divider(float left, float right, float y);
    void SidebarButton(int index, const wchar_t* glyph, float y);
    void ToggleRow(const std::wstring& label, bool& value, float left, float right, float y, bool dividerBelow = false);
    void SliderControl(const std::wstring& label, float& value, float minimum, float maximum,
        float left, float right, float y, ActiveSlider slider, int decimals = 1);
    void ComboControl(int id, const std::wstring& label, const std::vector<std::wstring>& items,
        int& selected, float left, float right, float y);
    void ColorRow(const std::wstring& label, D2D1_COLOR_F& color,
        float left, float right, float y);
    bool ActionButton(const std::wstring& label, const wchar_t* glyph,
        float left, float top, float right, float bottom, bool accent = false, bool danger = false);
    void TextInput(const std::wstring& label, std::wstring& value, bool& focused,
        float left, float right, float y);
    void StatusMessage(const std::wstring& text, bool success);
    void DrawComboPopup();
    void OpenColorPicker(D2D1_COLOR_F& color);
    void UpdatePickerColorFromHsv();
    void DrawColorPicker();

    std::filesystem::path ConfigDirectory() const;
    std::filesystem::path ConfigPath(const std::wstring& name) const;
    std::wstring SanitizeConfigName(const std::wstring& name) const;
    bool SaveConfig(const std::wstring& name);
    bool LoadConfig(const std::wstring& name);
    bool DeleteConfig(const std::wstring& name);
    void RefreshConfigs();
    void EnsureSelectedConfigVisible();
    bool HasSelectedConfig() const;
    std::wstring SelectedConfig() const;

    static std::wstring FormatNumber(float value, int decimals);
};
