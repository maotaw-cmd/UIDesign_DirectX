#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "PreviewRenderer.h"
#include "BeginnerCustomization.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using Microsoft::WRL::ComPtr;

static constexpr int CW = BeginnerCustomization::MainWindowWidth;
static constexpr int CH = BeginnerCustomization::MainWindowHeight;
static constexpr float SIDEBAR = BeginnerCustomization::SidebarWidth;
static constexpr float CONTENT_SHIFT = BeginnerCustomization::ContentShift;
static constexpr int PREVIEW_W = BeginnerCustomization::PreviewWindowWidth;
static constexpr int PREVIEW_H = CH;
static constexpr int PREVIEW_GAP = BeginnerCustomization::PreviewWindowGap;
static constexpr UINT_PTR PREVIEW_TIMER_ID = 77;

static constexpr int WINDOW_RADIUS = BeginnerCustomization::WindowCornerRadius;

static void ApplyRoundedWindowRegion(HWND window, int width, int height) {
    if (!window) return;
    HRGN region = CreateRoundRectRgn(
        0,
        0,
        width + 1,
        height + 1,
        WINDOW_RADIUS * 2,
        WINDOW_RADIUS * 2);
    if (region)
        SetWindowRgn(window, region, TRUE);
}

struct Hit { D2D1_RECT_F r{}; int id{}; };
struct RGB { float r=.95f, g=.35f, b=.35f; };

static HWND g_previewWindow = nullptr;
static HWND g_mainWindow = nullptr;

class App {
public:
    HWND hwnd{};
    ComPtr<ID2D1Factory> factory;
    ComPtr<ID2D1HwndRenderTarget> rt;
    ComPtr<IDWriteFactory> dw;
    ComPtr<IDWriteTextFormat> f11, f10, f9, f8, fBold;
    ComPtr<ID2D1SolidColorBrush> brush;

    int page = 0; // 0 Aimbot, 1 Visuals, 2 Misc, 3 Settings
    std::vector<Hit> hits;
    std::unordered_map<int, bool> checks;
    std::unordered_map<int, float> sliders;
    std::unordered_map<int, int> choices;
    std::unordered_map<int, std::wstring> hotkeys;
    std::unordered_map<int, RGB> colors;

    int openDropdown = -1;
    int openColor = -1;
    int colorTab = 0; // retained for config compatibility; visual picker is always used
    int captureHotkey = -1;
    int draggingSlider = -1;
    int draggingColorChannel = -1;
    bool drawingPage = false;
    float visualsScroll = 0.0f;
    bool draggingVisualScrollbar = false;
    float visualScrollbarDragOffset = 0.0f;
    int openVisualPopup = -1; // 0 box, 1 glow, 2 chams, 3 health, 4 armour
    int visualTarget = 0; // 0 enemy, 1 local

    static constexpr float VisualContentHeight = 336.0f;
    static constexpr float VisualViewportTop = 30.0f;
    static constexpr float VisualViewportBottom = 366.0f;

    std::vector<std::wstring> dropdownItems;
    D2D1_RECT_F dropdownAnchor{};

    std::wstring configName = L"default_config";
    std::vector<std::wstring> configFiles;
    int selectedConfig = -1;
    int configListScroll = 0;
    bool draggingConfigScrollbar = false;
    float configScrollbarDragOffset = 0.0f;
    bool configInputActive = false;

    #include "UI/DrawFunctions.inl"

    HRESULT Init(HWND h) {
        hwnd = h;
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return hr;
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dw.ReleaseAndGetAddressOf()));
        if (FAILED(hr)) return hr;

        auto mk = [&](float s, DWRITE_FONT_WEIGHT w, ComPtr<IDWriteTextFormat>& out) {
            return dw->CreateTextFormat(L"Segoe UI", nullptr, w, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, s, L"en-us", out.ReleaseAndGetAddressOf());
        };
        if (FAILED(mk(11.0f, DWRITE_FONT_WEIGHT_NORMAL, f11))) return E_FAIL;
        if (FAILED(mk(10.0f, DWRITE_FONT_WEIGHT_NORMAL, f10))) return E_FAIL;
        if (FAILED(mk(9.2f, DWRITE_FONT_WEIGHT_NORMAL, f9))) return E_FAIL;
        if (FAILED(mk(8.2f, DWRITE_FONT_WEIGHT_NORMAL, f8))) return E_FAIL;
        if (FAILED(mk(10.8f, DWRITE_FONT_WEIGHT_SEMI_BOLD, fBold))) return E_FAIL;

        for (int i=0;i<1000;i++) checks[i] = false;

        // Aimbot defaults
        checks[1] = true;   // enable aimbot
        checks[2] = true;   // visibility
        checks[4] = true;   // draw fov
        checks[5] = true;   // triggerbot
        checks[6] = true;   // recoil compensation
        checks[7] = true;   // link recoil
        sliders[200] = .62f; // smooth x
        sliders[201] = .56f; // smooth y
        sliders[202] = .40f; // fov
        sliders[203] = .72f; // hitchance
        sliders[204] = .20f; // first bullet delay
        choices[100] = 0;   // target bone
        hotkeys[300] = L"MB5";
        hotkeys[301] = L"ALT";
        colors[400] = {.95f,.34f,.35f};

        // Visuals defaults
        checks[10] = true;
        checks[11] = true;
        checks[12] = true;
        checks[13] = true;
        checks[14] = false; // armour disabled by default
        checks[15] = true;
        checks[17] = true;
        checks[18] = false; // snaplines disabled by default
        checks[19] = true;
        checks[20] = true;
        checks[21] = true;  // character chams

        // Local-player visuals are fully independent from enemy visuals.
        checks[50] = false; // local enable
        checks[51] = false; // local skeleton
        checks[52] = true;  // local name
        checks[53] = true;  // local health bar
        checks[54] = false; // local armour bar
        checks[55] = true;  // local weapon
        checks[56] = true;  // local distance
        checks[57] = false; // local snapline
        checks[58] = false; // local glow
        checks[59] = true;  // local chams

        choices[101] = 0; // enemy box style
        choices[121] = 0; // local box style
        choices[122] = 0; // local glow style
        choices[123] = 0; // local chams style
        choices[124] = 1; // local health style
        choices[125] = 1; // local armour style
        choices[126] = 0; // local health position
        choices[127] = 0; // local armour position
        sliders[215] = 0.10f; // local glow thickness
        sliders[216] = 1.0f / 11.0f; // local health width
        sliders[217] = 1.0f / 11.0f; // local armour width
        choices[102] = 0; // legacy fill style (unused)
        choices[103] = 0; // glow mode
        choices[104] = 0; // snapline anchor
        choices[105] = 1; // health style: segmented side
        choices[106] = 1; // armour style: segmented
        sliders[210] = .55f; // legacy radar size
        sliders[211] = .68f; // legacy radar opacity
        sliders[212] = BeginnerCustomization::DefaultBarWidthSlider; // health bar width: 2 px (range 1-12)
        sliders[213] = BeginnerCustomization::DefaultBarWidthSlider; // armour bar width: 2 px (range 1-12)
        sliders[214] = BeginnerCustomization::DefaultGlowThicknessSlider; // glow thickness: 0.7 px (range 0.5-2.5)
        choices[107] = 0; // chams style: solid
        colors[401] = {BeginnerCustomization::EnemyR, BeginnerCustomization::EnemyG, BeginnerCustomization::EnemyB}; // enemy box colour
        colors[402] = {BeginnerCustomization::LocalR, BeginnerCustomization::LocalG, BeginnerCustomization::LocalB};   // local box colour
        colors[416] = {0.94f, 0.30f, 0.33f}; // enemy filled colour
        colors[417] = {0.23f, 0.70f, 0.96f}; // local filled colour
        colors[418] = {0.95f, 0.34f, 0.35f}; // enemy glow colour
        colors[419] = {0.36f, 0.72f, 0.98f}; // local glow colour
        colors[403] = {.95f,.90f,.33f};   // enemy snapline
        colors[410] = {.30f,.78f,1.00f};   // local snapline
        colors[411] = {.36f,.88f,.52f};   // local health foreground
        colors[412] = {.08f,.20f,.12f};   // local health background
        colors[413] = {.24f,.62f,1.00f};   // local armour foreground
        colors[414] = {.04f,.12f,.24f};   // local armour background
        colors[415] = {.36f,.72f,.98f};   // local chams colour
        colors[404] = {.31f,.84f,.46f}; // health foreground
        colors[407] = {.70f,.086f,.133f}; // health background
        colors[408] = {.20f,.55f,1.00f}; // armour foreground
        colors[409] = {.035f,.10f,.22f}; // armour background
        colors[406] = {.63f,.66f,.72f}; // character chams colour

        // Misc defaults
        checks[31] = true; // no flash
        checks[32] = true; // no smoke
        checks[33] = true; // bhop
        checks[35] = true; // bomb esp
        checks[36] = true;
        checks[37] = true;
        choices[110] = 0;
        choices[111] = 0;
        hotkeys[302] = L"SPACE";
        hotkeys[303] = L"V";
        sliders[220] = .46f;

        // Settings defaults
        checks[40] = false; // removed
        checks[41] = false; // removed
        checks[42] = false; // removed
        checks[43] = true; // animations
        checks[44] = true; // tooltips
        choices[120] = 0; // config
        hotkeys[304] = L"INSERT";
        sliders[230] = BeginnerCustomization::DefaultWindowOpacity;
        colors[405] = {1.0f,1.0f,1.0f};

        RefreshConfigList();
        return S_OK;
    }

    HRESULT EnsureRT() {
        if (rt) return S_OK;
        RECT rc{}; GetClientRect(hwnd, &rc);
        HRESULT hr = factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right, rc.bottom)),
            rt.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return hr;
        return rt->CreateSolidColorBrush(C(0xffffff), brush.ReleaseAndGetAddressOf());
    }

    bool Inside(const D2D1_RECT_F& r, float x, float y) {
        return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
    }

    bool InteractiveAt(float x, float y) const {
        for (auto it = hits.rbegin(); it != hits.rend(); ++it) {
            if (x >= it->r.left && x <= it->r.right &&
                y >= it->r.top && y <= it->r.bottom)
                return true;
        }
        return false;
    }

    void HitBox(float l, float t, float r, float b, int id) {
        if (drawingPage) {
            l -= CONTENT_SHIFT;
            r -= CONTENT_SHIFT;
            if (page == 1) {
                t -= visualsScroll;
                b -= visualsScroll;
            }
        }
        hits.push_back({D2D1::RectF(l,t,r,b), id});
    }

    // Page implementations are split into beginner-friendly files.
    #include "Pages/AimbotPage.inl"
    #include "Pages/VisualsPage.inl"
    #include "Pages/MiscPage.inl"
    #include "Pages/SettingsPage.inl"

    static void RGBtoHSL(const RGB& c, float& h, float& s, float& l) {
        float mx = std::max(c.r, std::max(c.g, c.b));
        float mn = std::min(c.r, std::min(c.g, c.b));
        l = (mx + mn) / 2.0f;
        float d = mx - mn;
        if (d == 0.0f) { h = 0.0f; s = 0.0f; return; }
        s = l > 0.5f ? d / (2.0f - mx - mn) : d / (mx + mn);
        if (mx == c.r) h = (c.g - c.b) / d + (c.g < c.b ? 6.0f : 0.0f);
        else if (mx == c.g) h = (c.b - c.r) / d + 2.0f;
        else h = (c.r - c.g) / d + 4.0f;
        h /= 6.0f;
    }
    static float Hue2(float p, float q, float t) {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.f/6.f) return p + (q - p) * 6.f * t;
        if (t < .5f) return q;
        if (t < 2.f/3.f) return p + (q - p) * (2.f/3.f - t) * 6.f;
        return p;
    }
    static RGB HSLtoRGB(float h, float s, float l) {
        RGB c;
        if (s == 0) {
            c.r = c.g = c.b = l;
        } else {
            float q = l < .5f ? l * (1 + s) : l + s - l * s;
            float p = 2 * l - q;
            c.r = Hue2(p,q,h + 1.f/3.f);
            c.g = Hue2(p,q,h);
            c.b = Hue2(p,q,h - 1.f/3.f);
        }
        return c;
    }

    static void RGBtoHSV(const RGB& c, float& h, float& s, float& v) {
        const float mx = std::max(c.r, std::max(c.g, c.b));
        const float mn = std::min(c.r, std::min(c.g, c.b));
        const float d = mx - mn;
        v = mx;
        s = mx <= 0.00001f ? 0.0f : d / mx;
        if (d <= 0.00001f) {
            h = 0.0f;
            return;
        }
        if (mx == c.r)
            h = std::fmod((c.g - c.b) / d, 6.0f) / 6.0f;
        else if (mx == c.g)
            h = (((c.b - c.r) / d) + 2.0f) / 6.0f;
        else
            h = (((c.r - c.g) / d) + 4.0f) / 6.0f;
        if (h < 0.0f) h += 1.0f;
    }

    static RGB HSVtoRGB(float h, float s, float v) {
        h = h - std::floor(h);
        s = std::clamp(s, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        const float scaled = h * 6.0f;
        const int sector = static_cast<int>(std::floor(scaled)) % 6;
        const float f = scaled - std::floor(scaled);
        const float p = v * (1.0f - s);
        const float q = v * (1.0f - f * s);
        const float t = v * (1.0f - (1.0f - f) * s);
        switch (sector) {
        case 0: return {v, t, p};
        case 1: return {q, v, p};
        case 2: return {p, v, t};
        case 3: return {p, q, v};
        case 4: return {t, p, v};
        default: return {v, p, q};
        }
    }

    void OpenDrop(int id, const std::vector<std::wstring>& items,
                  const D2D1_RECT_F& r, bool screenSpace = false) {
        openDropdown = id;
        dropdownItems = items;
        dropdownAnchor = r;
        if (!screenSpace) {
            dropdownAnchor.left -= CONTENT_SHIFT;
            dropdownAnchor.right -= CONTENT_SHIFT;
            if (page == 1) {
                dropdownAnchor.top -= visualsScroll;
                dropdownAnchor.bottom -= visualsScroll;
            }
        }
        openColor = -1;
    }

    void HandleDropdownClick(int id) {
        switch (id) {
            case 100: OpenDrop(id, {L"Head",L"Neck",L"Chest",L"Nearest"}, D2D1::RectF(258,80,336,94)); break;
            case 101: OpenDrop(id, {L"Cornered",L"Box",L"Box + Filled",L"Cornered + Filled"}, D2D1::RectF(398,104,526,118), true); break;
            case 102: break;
            case 103: OpenDrop(id, {L"Soft",L"Pulse",L"Dynamic",L"Rainbow",L"Neon"}, D2D1::RectF(398,104,526,118), true); break;
            case 104: OpenDrop(id, {L"Head",L"Feet",L"Bottom"}, D2D1::RectF(466,168,545,182)); break;
            case 107: OpenDrop(id, {L"Solid",L"Pulse",L"Rainbow",L"Metallic"}, D2D1::RectF(398,104,526,118), true); break;
            case 105: OpenDrop(id, {L"Normal",L"Segmented",L"Gradient"}, D2D1::RectF(398,136,526,150), true); break;
            case 106: OpenDrop(id, {L"Normal",L"Segmented",L"Gradient"}, D2D1::RectF(398,136,526,150), true); break;
            case 108: OpenDrop(id, {L"Left",L"Right",L"Top",L"Bottom"}, D2D1::RectF(398,104,526,118), true); break;
            case 109: OpenDrop(id, {L"Left",L"Right",L"Top",L"Bottom"}, D2D1::RectF(398,104,526,118), true); break;
            case 110: OpenDrop(id, {L"Bayonet",L"Karambit",L"Butterfly",L"M9"}, D2D1::RectF(278,80,336,94)); break;
            case 111: OpenDrop(id, {L"Sport",L"Driver",L"Moto",L"Wraps"}, D2D1::RectF(278,102,336,116)); break;
            case 120: OpenDrop(id, {L"Default",L"Legit",L"Rage",L"Visuals",L"Custom"}, D2D1::RectF(244,58,336,72)); break;
            case 121: OpenDrop(id, {L"Cornered",L"Box",L"Box + Filled",L"Cornered + Filled"}, D2D1::RectF(398,104,526,118), true); break;
            case 122: OpenDrop(id, {L"Soft",L"Pulse",L"Dynamic",L"Rainbow",L"Neon"}, D2D1::RectF(398,104,526,118), true); break;
            case 123: OpenDrop(id, {L"Solid",L"Pulse",L"Rainbow",L"Metallic"}, D2D1::RectF(398,104,526,118), true); break;
            case 124: OpenDrop(id, {L"Normal",L"Segmented",L"Gradient"}, D2D1::RectF(398,136,526,150), true); break;
            case 125: OpenDrop(id, {L"Normal",L"Segmented",L"Gradient"}, D2D1::RectF(398,136,526,150), true); break;
            case 126: OpenDrop(id, {L"Left",L"Right",L"Top",L"Bottom"}, D2D1::RectF(398,104,526,118), true); break;
            case 127: OpenDrop(id, {L"Left",L"Right",L"Top",L"Bottom"}, D2D1::RectF(398,104,526,118), true); break;
            default: break;
        }
    }

    void UpdateSliderFromMouse(float screenX) {
        const bool popupSlider = openVisualPopup >= 0 &&
            (draggingSlider == 212 || draggingSlider == 213 || draggingSlider == 214 ||
             draggingSlider == 215 || draggingSlider == 216 || draggingSlider == 217);
        float x = popupSlider ? screenX : screenX + CONTENT_SHIFT;
        float sx = 160.0f, w = 176.0f;
        switch (draggingSlider) {
            case 200: case 201: case 202: case 220:
                sx = 160.0f; w = 176.0f; break;
            case 203: case 204: case 210: case 211: case 230:
                sx = 364.0f; w = 180.0f; break;
            case 212: case 213: case 214: case 215: case 216: case 217:
                if (popupSlider) { sx = 398.0f; w = 128.0f; }
                else { sx = 452.0f; w = 93.0f; }
                break;
            default: sx = 160.0f; w = 176.0f; break;
        }
        sliders[draggingSlider] = std::clamp((x - sx) / w, 0.0f, 1.0f);

        // Transparency must update immediately while the thumb is being dragged.
        if (draggingSlider == 230)
            ApplyWindowAppearance();

        InvalidateRect(hwnd, nullptr, FALSE);
        if (g_previewWindow)
            InvalidateRect(g_previewWindow, nullptr, FALSE);
    }

    void UpdateColorDrag(float screenX, float screenY) {
        if (openColor < 0) return;

        RGB current = colors[openColor];
        float hue = 0.0f, saturation = 0.0f, value = 1.0f;
        RGBtoHSV(current, hue, saturation, value);

        const float popupX = 323.0f;
        const float popupY = 54.0f;
        const float svX = popupX + 12.0f;
        const float svY = popupY + 34.0f;
        const float svW = 166.0f;
        const float svH = 136.0f;
        const float hueY = svY;
        const float hueH = svH;

        if (draggingColorChannel == 6) {
            saturation = std::clamp((screenX - svX) / svW, 0.0f, 1.0f);
            value = 1.0f - std::clamp((screenY - svY) / svH, 0.0f, 1.0f);
        } else if (draggingColorChannel == 7) {
            hue = std::clamp((screenY - hueY) / hueH, 0.0f, 0.999999f);
        }

        colors[openColor] = HSVtoRGB(hue, saturation, value);
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void MouseDown(float x, float y) {
        SetCapture(hwnd);
        const float visualMaximumScroll =
            std::max(0.0f, VisualContentHeight - (VisualViewportBottom - VisualViewportTop));
        if (page == 1 && visualMaximumScroll > 0.0f && x >= CW - 9.0f) {
            const float trackTop = VisualViewportTop + 3.0f;
            const float trackBottom = VisualViewportBottom - 3.0f;
            const float trackHeight = trackBottom - trackTop;
            const float viewportHeight = VisualViewportBottom - VisualViewportTop;
            const float thumbHeight = std::max(42.0f, trackHeight * (viewportHeight / VisualContentHeight));
            const float maximumScroll = std::max(0.0f, VisualContentHeight - viewportHeight);
            const float available = trackHeight - thumbHeight;
            const float thumbTop = trackTop + available * (maximumScroll > 0.0f ? visualsScroll / maximumScroll : 0.0f);
            if (y >= thumbTop && y <= thumbTop + thumbHeight)
                visualScrollbarDragOffset = y - thumbTop;
            else
                visualScrollbarDragOffset = thumbHeight * 0.5f;
            draggingVisualScrollbar = true;
            UpdateVisualScrollbarFromMouse(y);
            return;
        }
        if (openVisualPopup >= 0) {
            const float popupHeight =
                openVisualPopup == 0 ? 146.0f :
                ((openVisualPopup == 3 || openVisualPopup == 4) ? 210.0f : 176.0f);
            const bool insidePopup = x >= 286.0f && x <= 544.0f &&
                                     y >= 62.0f && y <= 62.0f + popupHeight;
            if (!insidePopup && openDropdown < 0 && openColor < 0) {
                openVisualPopup = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
                ReleaseCapture();
                return;
            }
        }

        if (page == 3 && x >= 329.0f && x <= 342.0f &&
            y >= 160.0f && y <= 267.0f &&
            static_cast<int>(configFiles.size()) > 5) {
            constexpr int visible = 5;
            constexpr float trackTop = 160.0f;
            constexpr float trackBottom = 267.0f;
            const float trackHeight = trackBottom - trackTop;
            const float thumbHeight = std::max(
                24.0f,
                trackHeight * (static_cast<float>(visible) /
                               static_cast<float>(configFiles.size())));
            const int maximumScroll =
                std::max(0, static_cast<int>(configFiles.size()) - visible);
            const float available = trackHeight - thumbHeight;
            const float thumbTop = trackTop + available *
                (static_cast<float>(configListScroll) /
                 static_cast<float>(maximumScroll));

            if (y >= thumbTop && y <= thumbTop + thumbHeight)
                configScrollbarDragOffset = y - thumbTop;
            else
                configScrollbarDragOffset = thumbHeight * 0.5f;

            draggingConfigScrollbar = true;
            UpdateConfigScrollbarFromMouse(y);
            return;
        }

        for (auto it = hits.rbegin(); it != hits.rend(); ++it) {
            if (!Inside(it->r, x, y)) continue;
            int id = it->id;

            if (id == 8100 || id == 8101) {
                visualTarget = id - 8100;
                openVisualPopup = -1;
                openDropdown = -1;
                openColor = -1;
                InvalidateRect(hwnd, nullptr, TRUE);
                if (g_previewWindow) InvalidateRect(g_previewWindow, nullptr, FALSE);
                return;
            }
            if (id >= 8000 && id <= 8004) {
                openVisualPopup = id - 8000;
                openDropdown = -1;
                openColor = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id == 8099) {
                openVisualPopup = -1;
                openDropdown = -1;
                openColor = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 7000 && id <= 7003) {
                page = id - 7000;
                openDropdown = openColor = -1;
                openVisualPopup = -1;
                visualsScroll = 0.0f;
                InvalidateRect(hwnd, nullptr, TRUE);
                UpdateWindow(hwnd);
                return;
            }
            if (id == 7100) { ShowWindow(hwnd, SW_MINIMIZE); return; }
            if (id == 7101) { DestroyWindow(hwnd); return; }
            if (id == 7999) { openColor = -1; InvalidateRect(hwnd, nullptr, FALSE); return; }
            if (id == 7200 || id == 7201) {
                colorTab = id - 7200;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if ((id == 7216 || id == 7217) && openColor >= 0) {
                draggingColorChannel = id == 7216 ? 6 : 7;
                UpdateColorDrag(x, y);
                return;
            }
            if (id >= 7300 && id < 7307 && openColor >= 0) {
                unsigned p[] = {0xF05A5A,0xF4A62A,0xF3DD4F,0x53D76A,0x43BEEA,0x9B62E8,0xE164C7};
                unsigned v = p[id - 7300];
                colors[openColor] = {((v>>16)&255)/255.f, ((v>>8)&255)/255.f, (v&255)/255.f};
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 7400 && id < 7420 && openDropdown >= 0) {
                choices[openDropdown] = id - 7400;
                openDropdown = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id == 7600) {
                configInputActive = true;
                SetFocus(hwnd);
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 7700 && id < 7800) {
                const int index = id - 7700;
                if (index >= 0 && index < static_cast<int>(configFiles.size())) {
                    selectedConfig = index;
                    configName = configFiles[static_cast<std::size_t>(index)];
                    configInputActive = false;
                    const int maximumScroll =
                        std::max(0, static_cast<int>(configFiles.size()) - 5);
                    if (selectedConfig < configListScroll)
                        configListScroll = selectedConfig;
                    else if (selectedConfig >= configListScroll + 5)
                        configListScroll = selectedConfig - 4;
                    configListScroll =
                        std::clamp(configListScroll, 0, maximumScroll);
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return;
            }
            if (id == 6400) {
                SaveNamedConfig();
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id == 6401) {
                LoadNamedConfig();
                return;
            }
            if (id == 6403) {
                DeleteNamedConfig();
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 5500 && id < 7000) {
                MessageBeep(MB_OK);
                return;
            }
            if (id >= 4500 && id < 5500) {
                HandleDropdownClick(id - 4500);
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 3500 && id < 4500) {
                captureHotkey = id - 3500;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id >= 2500 && id < 3500) {
                draggingSlider = id - 2500;
                UpdateSliderFromMouse(x);
                return;
            }
            if (id >= 1500 && id < 2500) {
                openColor = id - 1500;
                openDropdown = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
            if (id < 500) {
                checks[id] = !checks[id];
                InvalidateRect(hwnd, nullptr, FALSE);
                return;
            }
        }

        if (openDropdown >= 0) {
            openDropdown = -1;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (openColor >= 0 && !(x >= 323 && x <= 547 && y >= 54 && y <= 312)) {
            openColor = -1;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }

    void MouseMove(float x, float y) {
        if (draggingConfigScrollbar) {
            UpdateConfigScrollbarFromMouse(y);
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return;
        }
        if (draggingVisualScrollbar) {
            UpdateVisualScrollbarFromMouse(y);
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return;
        }
        if (draggingSlider >= 0) UpdateSliderFromMouse(x);
        if (draggingColorChannel >= 0) UpdateColorDrag(x, y);
        const bool overVisualScrollbar = page == 1 && x >= CW - 9.0f;
        const bool overConfigScrollbar =
            page == 3 && static_cast<int>(configFiles.size()) > 5 &&
            x >= 329.0f && x <= 342.0f && y >= 160.0f && y <= 267.0f;
        SetCursor(LoadCursorW(nullptr,
            (overVisualScrollbar || overConfigScrollbar || InteractiveAt(x,y))
                ? IDC_HAND : IDC_ARROW));
    }

    void MouseUp() {
        draggingSlider = -1;
        draggingColorChannel = -1;
        draggingVisualScrollbar = false;
        draggingConfigScrollbar = false;
        ReleaseCapture();
    }

    void MouseWheel(short delta) {
        if (openColor >= 0 || openDropdown >= 0) return;

        const int steps = delta / WHEEL_DELTA;
        if (page == 3 && static_cast<int>(configFiles.size()) > 5) {
            const int maximumScroll =
                std::max(0, static_cast<int>(configFiles.size()) - 5);
            configListScroll = std::clamp(
                configListScroll - steps, 0, maximumScroll);
            InvalidateRect(hwnd, nullptr, FALSE);
            return;
        }

        if (page == 1) {
            const float wheelSteps =
                static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
            SetVisualScroll(visualsScroll - wheelSteps * 48.0f);
        }
    }


    void ApplyWindowAppearance() {
        const float opacity = std::clamp(sliders[230], 0.25f, 1.0f);
        const BYTE alpha = static_cast<BYTE>(std::lround(opacity * 255.0f));
        auto apply = [&](HWND window) {
            if (!window) return;
            LONG_PTR exStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
            if ((exStyle & WS_EX_LAYERED) == 0)
                SetWindowLongPtrW(window, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
            SetLayeredWindowAttributes(window, 0, alpha, LWA_ALPHA);
        };
        apply(g_mainWindow);
        apply(g_previewWindow);
    }

    void SyncPreviewSettings() {
        PreviewRenderer::Settings s{};

        const bool local = visualTarget == 1;
        const int enableId = local ? 50 : 10;
        const int skeletonId = local ? 51 : 11;
        const int nameId = local ? 52 : 12;
        const int healthId = local ? 53 : 13;
        const int armorId = local ? 54 : 14;
        const int weaponId = local ? 55 : 15;
        const int distanceId = local ? 56 : 16;
        const int snaplineId = local ? 57 : 18;
        const int glowId = local ? 58 : 17;
        const int chamsId = local ? 59 : 21;

        const int boxStyle = choices[local ? 121 : 101];
        const bool visualsEnabled = checks[enableId];

        s.characterScale = BeginnerCustomization::CharacterScale;
        s.particles = false;
        s.particleSpeed = 22.0f;
        s.particleAmount = 30.0f;

        s.box = visualsEnabled;
        s.cornerBox = visualsEnabled && (boxStyle == 0 || boxStyle == 3);
        s.filled = visualsEnabled && (boxStyle == 2 || boxStyle == 3);
        s.healthBar = visualsEnabled && checks[healthId];
        s.armorBar = visualsEnabled && checks[armorId];
        s.name = visualsEnabled && checks[nameId];
        s.distance = visualsEnabled && checks[distanceId];
        s.weapon = visualsEnabled && checks[weaponId];
        s.snapline = visualsEnabled && checks[snaplineId];
        s.skeleton = visualsEnabled && checks[skeletonId];
        s.damage = visualsEnabled;

        s.visualGlow = checks[glowId];
        s.characterChams = checks[chamsId];
        s.visualGlowMode = std::clamp(choices[local ? 122 : 103], 0, 4);
        s.visualGlowThickness = 0.5f +
            std::clamp(sliders[local ? 215 : 214], 0.0f, 1.0f) * 2.0f;
        s.characterChamsStyle = std::clamp(choices[local ? 123 : 107], 0, 3);
        s.healthStyle = std::clamp(choices[local ? 124 : 105], 0, 2);
        s.armorStyle = std::clamp(choices[local ? 125 : 106], 0, 2);
        s.healthPosition = std::clamp(choices[local ? 126 : 108], 0, 3);
        s.armorPosition = std::clamp(choices[local ? 127 : 109], 0, 3);
        s.healthBarWidth = 1.0f +
            std::clamp(sliders[local ? 216 : 212], 0.0f, 1.0f) * 11.0f;
        s.armorBarWidth = 1.0f +
            std::clamp(sliders[local ? 217 : 213], 0.0f, 1.0f) * 11.0f;

        const RGB target = colors[local ? 402 : 401];
        const RGB filledTarget = colors[local ? 417 : 416];
        const RGB glowTarget = colors[local ? 419 : 418];
        const RGB health = colors[local ? 411 : 404];
        const RGB healthBack = colors[local ? 412 : 407];
        const RGB armor = colors[local ? 413 : 408];
        const RGB armorBack = colors[local ? 414 : 409];
        const RGB chams = colors[local ? 415 : 406];

        const RGB accent = colors[405];
        s.accent[0] = accent.r; s.accent[1] = accent.g; s.accent[2] = accent.b; s.accent[3] = 1.0f;

        s.boxColour[0] = target.r;
        s.boxColour[1] = target.g;
        s.boxColour[2] = target.b;
        s.boxColour[3] = 1.0f;

        s.filledColour[0] = filledTarget.r;
        s.filledColour[1] = filledTarget.g;
        s.filledColour[2] = filledTarget.b;
        s.filledColour[3] = 0.22f;

        s.healthColour[0] = health.r;
        s.healthColour[1] = health.g;
        s.healthColour[2] = health.b;
        s.healthColour[3] = 1.0f;

        s.healthBackColour[0] = healthBack.r;
        s.healthBackColour[1] = healthBack.g;
        s.healthBackColour[2] = healthBack.b;
        s.healthBackColour[3] = 1.0f;

        s.armorColour[0] = armor.r;
        s.armorColour[1] = armor.g;
        s.armorColour[2] = armor.b;
        s.armorColour[3] = 1.0f;

        s.armorBackColour[0] = armorBack.r;
        s.armorBackColour[1] = armorBack.g;
        s.armorBackColour[2] = armorBack.b;
        s.armorBackColour[3] = 1.0f;

        s.visualGlowColour[0] = glowTarget.r;
        s.visualGlowColour[1] = glowTarget.g;
        s.visualGlowColour[2] = glowTarget.b;
        s.visualGlowColour[3] = 1.0f;

        s.lineColour[0] = colors[local ? 410 : 403].r;
        s.lineColour[1] = colors[local ? 410 : 403].g;
        s.lineColour[2] = colors[local ? 410 : 403].b;
        s.lineColour[3] = 1.0f;

        s.characterColour[0] = chams.r;
        s.characterColour[1] = chams.g;
        s.characterColour[2] = chams.b;
        s.characterColour[3] = 1.0f;

        s.backgroundColour[0] = 0x11 / 255.0f;
        s.backgroundColour[1] = 0x14 / 255.0f;
        s.backgroundColour[2] = 0x16 / 255.0f;
        s.backgroundColour[3] = 1.0f;

        PreviewRenderer::SetSettings(s);
        ApplyWindowAppearance();
    }

    void CharInput(wchar_t character) {
        if (!configInputActive) return;
        if (character == L'\b') {
            if (!configName.empty()) configName.pop_back();
        } else if (character == L'\r') {
            SaveNamedConfig();
            configInputActive = false;
        } else if (character >= 32 && (iswalnum(character) || character == L'_' || character == L'-')) {
            if (configName.size() < 32) configName.push_back(character);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void KeyDown(WPARAM key) {
        if (captureHotkey >= 0) {
            wchar_t name[32] = L"";
            UINT scan = MapVirtualKeyW((UINT)key, MAPVK_VK_TO_VSC) << 16;
            if (key == VK_LEFT || key == VK_UP || key == VK_RIGHT || key == VK_DOWN ||
                key == VK_PRIOR || key == VK_NEXT || key == VK_END || key == VK_HOME ||
                key == VK_INSERT || key == VK_DELETE) {
                scan |= 1 << 24;
            }
            GetKeyNameTextW((LONG)scan, name, 32);
            hotkeys[captureHotkey] = name[0] ? name : L"KEY";
            captureHotkey = -1;
            InvalidateRect(hwnd, nullptr, FALSE);
            return;
        }
        if (key == VK_ESCAPE) {
            if (configInputActive) {
                configInputActive = false;
                InvalidateRect(hwnd, nullptr, FALSE);
            } else if (openColor >= 0 || openDropdown >= 0) {
                openColor = openDropdown = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
            } else {
                DestroyWindow(hwnd);
            }
        }
    }
};

static App* g_app = nullptr;

void PositionPreviewWindow() {
    if (!g_mainWindow || !g_previewWindow) return;
    RECT r{};
    GetWindowRect(g_mainWindow, &r);
    SetWindowPos(
        g_previewWindow,
        HWND_TOP,
        r.right + PREVIEW_GAP,
        r.top,
        PREVIEW_W,
        PREVIEW_H,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ApplyRoundedWindowRegion(g_previewWindow, PREVIEW_W, PREVIEW_H);
}

LRESULT CALLBACK PreviewWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    bool handled = false;
    const LRESULT result = PreviewRenderer::HandleMessage(h, m, w, l, handled);
    if (handled) return result;

    switch (m) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        BeginPaint(h, &ps);
        if (g_app) g_app->SyncPreviewSettings();
        PreviewRenderer::Render();
        EndPaint(h, &ps);
        return 0;
    }
    case WM_NCHITTEST:
        return HTCLIENT;
    }
    return DefWindowProcW(h, m, w, l);
}


LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        BeginPaint(h, &ps);
        if (g_app) g_app->Paint();
        EndPaint(h, &ps);
        return 0;
    }
    case WM_SIZE:
        if (g_app && g_app->rt) g_app->rt->Resize(D2D1::SizeU(LOWORD(l), HIWORD(l)));
        ApplyRoundedWindowRegion(h, LOWORD(l), HIWORD(l));
        PositionPreviewWindow();
        return 0;
    case WM_MOVE:
        PositionPreviewWindow();
        return 0;
    case WM_TIMER:
        if (w == PREVIEW_TIMER_ID && g_previewWindow) {
            if (g_app) g_app->SyncPreviewSettings();
            InvalidateRect(g_previewWindow, nullptr, FALSE);
        }
        return 0;
    case WM_LBUTTONDOWN:
        if (g_app) g_app->MouseDown((float)GET_X_LPARAM(l), (float)GET_Y_LPARAM(l));
        return 0;
    case WM_MOUSEMOVE:
        if (g_app) g_app->MouseMove((float)GET_X_LPARAM(l), (float)GET_Y_LPARAM(l));
        return 0;
    case WM_LBUTTONUP:
        if (g_app) g_app->MouseUp();
        return 0;
    case WM_MOUSEWHEEL:
        if (g_app) g_app->MouseWheel(GET_WHEEL_DELTA_WPARAM(w));
        return 0;
    case WM_KEYDOWN:
        if (g_app) g_app->KeyDown(w);
        return 0;
    case WM_CHAR:
        if (g_app) g_app->CharInput(static_cast<wchar_t>(w));
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(l) == HTCLIENT && g_app) {
            POINT p{}; GetCursorPos(&p); ScreenToClient(h, &p);
            SetCursor(LoadCursorW(nullptr,
                g_app->InteractiveAt((float)p.x, (float)p.y)
                    ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;
    case WM_NCHITTEST: {
        POINT p{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
        ScreenToClient(h, &p);
        if (p.y < 34 && p.x > SIDEBAR && p.x < CW-46) return HTCAPTION;
        return HTCLIENT;
    }
    case WM_DESTROY:
        KillTimer(h, PREVIEW_TIMER_ID);
        if (g_previewWindow) {
            PreviewRenderer::Shutdown();
            DestroyWindow(g_previewWindow);
            g_previewWindow = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, LPWSTR, int) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"ArcaneD2DMenuV6";
    RegisterClassExW(&wc);

    WNDCLASSEXW previewClass{};
    previewClass.cbSize = sizeof(previewClass);
    previewClass.style = CS_HREDRAW | CS_VREDRAW;
    previewClass.lpfnWndProc = PreviewWndProc;
    previewClass.hInstance = hi;
    previewClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    previewClass.lpszClassName = L"Arcane3DPreviewWindow";
    RegisterClassExW(&previewClass);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    HWND h = CreateWindowExW(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        L"Arcane",
        WS_POPUP,
        (sw - (CW + PREVIEW_GAP + PREVIEW_W)) / 2,
        (sh - CH) / 2,
        CW,
        CH,
        nullptr,
        nullptr,
        hi,
        nullptr);

    if (!h) {
        CoUninitialize();
        return 0;
    }

    g_mainWindow = h;
    ApplyRoundedWindowRegion(h, CW, CH);

    RECT mainRect{};
    GetWindowRect(h, &mainRect);
    g_previewWindow = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        previewClass.lpszClassName,
        L"Preview",
        WS_POPUP,
        mainRect.right + PREVIEW_GAP,
        mainRect.top,
        PREVIEW_W,
        PREVIEW_H,
        h,
        nullptr,
        hi,
        nullptr);

    ApplyRoundedWindowRegion(g_previewWindow, PREVIEW_W, PREVIEW_H);

    if (!g_previewWindow || !PreviewRenderer::Initialize(g_previewWindow)) {
        if (g_previewWindow) DestroyWindow(g_previewWindow);
        DestroyWindow(h);
        g_previewWindow = nullptr;
        g_mainWindow = nullptr;
        CoUninitialize();
        return 0;
    }

    App app;
    g_app = &app;
    if (FAILED(app.Init(h))) {
        g_app = nullptr;
        CoUninitialize();
        return 0;
    }

    app.ApplyWindowAppearance();

    ShowWindow(h, SW_SHOW);
    ShowWindow(g_previewWindow, SW_SHOWNOACTIVATE);
    UpdateWindow(h);
    UpdateWindow(g_previewWindow);
    SetTimer(h, PREVIEW_TIMER_ID, 16, nullptr);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    g_app = nullptr;
    g_mainWindow = nullptr;
    CoUninitialize();
    return (int)msg.wParam;
}