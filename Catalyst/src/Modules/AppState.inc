


// Application state, enums, geometry helpers, mouse state and shared values.
// Included once through ProjectAll.h. Do not add this file to Source Files.

    constexpr int WINDOW_W = 960;
    constexpr int WINDOW_H = 620;

    constexpr float TOP_H = 64.0f;
    constexpr float PAD = 14.0f;
    constexpr float GAP = 12.0f;
    constexpr float LEFT_W = 390.0f;

    // Theme, colours, mesh types, and visual defaults are in CustomizationUI.h.

    enum class SliderId
    {
        None,
        BoxThickness,
        BoxFillAlpha,
        HealthBarThickness,
        HealthOutlineThickness,
        SnaplineThickness,
        SkeletonThickness,
        VisualGlowThickness,
        SoundWalkSpeed,
        SoundWalkExpansion,
        SoundWalkThickness,
        ModelScale,
        ParticleSpeed,
        ParticleAmount
    };

    enum class VisualPopup
    {
        None,
        Box,
        HealthBar,
        Snapline,
        Skeleton,
        SoundWalk,
        ModelOutline
    };

    enum class PopupDropdown
    {
        None,
        BoxStyle,
        HealthStyle,
        SnaplineOrigin,
        SoundWalkAnimation
    };

    struct Slider
    {
        SliderId id = SliderId::None;
        D2D1_RECT_F hit{};
        float* value = nullptr;
        float minimum = 0.0f;
        float maximum = 1.0f;
    };

    struct App
    {
        HWND hwnd = nullptr;
        UINT width = WINDOW_W;
        UINT height = WINDOW_H;

        // D3D
        ComPtr<ID3D11Device> d3dDevice;
        ComPtr<ID3D11DeviceContext> d3dContext;
        ComPtr<IDXGISwapChain1> swapChain;
        ComPtr<ID3D11RenderTargetView> renderTargetView;
        ComPtr<ID3D11Texture2D> depthTexture;
        ComPtr<ID3D11DepthStencilView> depthView;
        ComPtr<ID3D11VertexShader> vertexShader;
        ComPtr<ID3D11PixelShader> pixelShader;
        ComPtr<ID3D11InputLayout> inputLayout;
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
        ComPtr<ID3D11Buffer> constantBuffer;
        UINT embeddedIndexCount = 0;
        ComPtr<ID3D11RasterizerState> rasterizerState;
        ComPtr<ID3D11RasterizerState> outlineRasterizerState;
        ComPtr<ID3D11BlendState> alphaBlend;
        ComPtr<ID3D11BlendState> colourWriteDisabledBlend;
        ComPtr<ID3D11DepthStencilState> depthState;
        ComPtr<ID3D11DepthStencilState> outlineDepthState;

        // D2D/DWrite
        ComPtr<ID2D1Factory1> d2dFactory;
        ComPtr<ID2D1Device> d2dDevice;
        ComPtr<ID2D1DeviceContext> d2dContext;
        ComPtr<ID2D1Bitmap1> d2dTarget;
        ComPtr<ID2D1SolidColorBrush> brush;
        ComPtr<IDWriteFactory> writeFactory;
        ComPtr<IDWriteTextFormat> text12;
        ComPtr<IDWriteTextFormat> text13;
        ComPtr<IDWriteTextFormat> text14;
        ComPtr<IDWriteTextFormat> text16Bold;
        ComPtr<IDWriteTextFormat> text19Bold;
        ComPtr<IDWriteTextFormat> text27Bold;
        ComPtr<IDWriteTextFormat> iconFont;

        VisualSettings settings;

        bool settingsOpen = false;
        bool particlesEnabled = true;
        float particleSpeed = 22.0f;
        float particleAmount = 34.0f;

        // Automatic preview damage animation. This is intentionally not user controlled.
        float previewHealth = 100.0f;
        float previewHealthTarget = 100.0f;
        float healthAnimationTimer = 0.70f;
        int healthDamageStep = 0;
        float damageTextTimer = 0.0f;
        int damageTextValue = 0;

        // Preview value. In a real integration, assign the local/entity weapon name here.
        std::wstring previewWeaponName = L"AK-47";

        // Small per-visual settings cards opened by the gear beside each paint icon.
        VisualPopup visualPopup = VisualPopup::None;
        D2D1_RECT_F visualPopupRect{};
        float visualPopupAnchorY = 0.0f;
        float visualPopupX = 0.0f;
        float visualPopupY = 0.0f;
        bool visualPopupPositioned = false;
        bool draggingVisualPopup = false;
        float visualPopupDragOffsetX = 0.0f;
        float visualPopupDragOffsetY = 0.0f;

        // Optional per-visual hotkeys. 0 means Always on / no key required.
        // Indexed by the VisualPopup enum value.
        int visualHotkeyVk[7]{};
        bool visualHotkeyCtrl[7]{};
        bool capturingVisualHotkey = false;
        VisualPopup hotkeyCapturePopup = VisualPopup::None;

        // Reused by the style/origin dropdowns inside the small settings cards.
        bool healthStyleDropdownOpen = false;
        PopupDropdown popupDropdown = PopupDropdown::None;
        D2D1_RECT_F healthStyleButtonRect{};
        D2D1_RECT_F healthStylePopupRect{};

        std::wstring saveStatus;
        float saveStatusTimer = 0.0f;

        // Named configuration manager opened by the top save icon.
        bool configOpen = false;
        std::wstring configName;
        bool configNameFocused = false;
        std::vector<std::wstring> configs;
        int selectedConfig = -1;
        float configScroll = 0.0f;

        // Smooth scrolling for the left visuals list.
        float visualScroll = 0.0f;
        float visualScrollTarget = 0.0f;
        float visualScrollMax = 0.0f;
        D2D1_RECT_F visualScrollRect{};

        // Per-toggle animation progress (0 = left/off, 1 = right/on).
        std::unordered_map<const bool*, float> toggleAnimations;
        std::wstring configStatus;
        bool configStatusOk = true;
        float configStatusTimer = 0.0f;
        D2D1_RECT_F configPanelRect{};
        D2D1_RECT_F configInputRect{};
        bool configPanelPositioned = false;
        bool draggingConfigPanel = false;
        float configPanelX = 0.0f;
        float configPanelY = 0.0f;
        float configPanelDragOffsetX = 0.0f;
        float configPanelDragOffsetY = 0.0f;

        // Rebuilt every frame and used to show a hand cursor over controls.
        std::vector<D2D1_RECT_F> interactiveRects;

        POINT mouse{ -1000, -1000 };
        bool mouseDown = false;
        bool clicked = false;
        bool clickConsumed = false;

        SliderId activeSlider = SliderId::None;
        std::vector<Slider> sliders;

        bool colourPickerOpen = false;
        D2D1_COLOR_F* colourTarget = nullptr;
        D2D1_COLOR_F originalColour{};
        D2D1_COLOR_F workingColour{};
        float pickerHue = 0.0f;
        float pickerSaturation = 0.0f;
        float pickerValue = 1.0f;
        int activePickerArea = -1;

        bool draggingModel = false;
        float modelYaw = -0.45f;
        float previousMouseX = 0.0f;

        D2D1_RECT_F previewRect{};
    } app;

    D2D1_RECT_F Rect(float l, float t, float r, float b)
    {
        return D2D1::RectF(l, t, r, b);
    }

    void RegisterInteractive(const D2D1_RECT_F& rect)
    {
        app.interactiveRects.push_back(rect);
    }

    bool IsInteractiveAt(float x, float y)
    {
        for (const D2D1_RECT_F& rect : app.interactiveRects)
        {
            if (Hit(x, y, rect))
                return true;
        }
        return false;
    }

    void UpdatePointerCursor()
    {
        const float x = static_cast<float>(app.mouse.x);
        const float y = static_cast<float>(app.mouse.y);

        if (app.configOpen && Hit(x, y, app.configInputRect))
        {
            SetCursor(LoadCursorW(nullptr, IDC_IBEAM));
            return;
        }

        SetCursor(LoadCursorW(nullptr, IsInteractiveAt(x, y) ? IDC_HAND : IDC_ARROW));
    }

    // ------------------------------------------------------------------------
    // Colour conversion
    // ------------------------------------------------------------------------

    D2D1_COLOR_F HsvToColour(float hue, float saturation, float value)
    {
        hue -= std::floor(hue);
        saturation = Clamp(saturation, 0.0f, 1.0f);
        value = Clamp(value, 0.0f, 1.0f);

        const float scaled = hue * 6.0f;
        const int sector = static_cast<int>(std::floor(scaled)) % 6;
        const float fraction = scaled - std::floor(scaled);
        const float p = value * (1.0f - saturation);
        const float q = value * (1.0f - fraction * saturation);
        const float t = value * (1.0f - (1.0f - fraction) * saturation);

        switch (sector)
        {
        case 0: return D2D1::ColorF(value, t, p, 1.0f);
        case 1: return D2D1::ColorF(q, value, p, 1.0f);
        case 2: return D2D1::ColorF(p, value, t, 1.0f);
        case 3: return D2D1::ColorF(p, q, value, 1.0f);
        case 4: return D2D1::ColorF(t, p, value, 1.0f);
        default:return D2D1::ColorF(value, p, q, 1.0f);
        }
    }

