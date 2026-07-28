#pragma once

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <urlmon.h>

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "../resource.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

constexpr int WINDOW_WIDTH = 926;
constexpr int WINDOW_HEIGHT = 582;
constexpr float SIDEBAR_WIDTH = 0.0f;

extern HWND g_hwnd;

extern ID2D1Factory* g_d2d;
extern ID2D1HwndRenderTarget* g_rt;
extern ID2D1SolidColorBrush* g_brush;
extern ID2D1GradientStopCollection* g_stops;
extern ID2D1LinearGradientBrush* g_gradient;

extern IDWriteFactory* g_dw;
extern IDWriteTextFormat* g_font;
extern IWICImagingFactory* g_wic;

extern ID2D1Bitmap* g_inputBitmap;
extern ID2D1Bitmap* g_outputBitmap;

extern std::wstring g_inputPath;
extern std::wstring g_outputPath;
extern std::wstring g_status;

extern std::atomic<bool> g_ready;
extern std::atomic<bool> g_busy;
extern std::atomic<float> g_progress;
extern std::atomic<float> g_progressTarget;
extern std::atomic<int> g_processPercent;

extern float g_mouseX;
extern float g_mouseY;
extern float g_resultAnim;
extern float g_borderPhase;

extern bool g_showSettings;
extern bool g_eraserMode;
extern bool g_dragSlider;
extern bool g_isErasing;
extern float g_featherValue;

extern std::vector<BYTE> g_outputPixels;
extern UINT g_outputPixelWidth;
extern UINT g_outputPixelHeight;
extern UINT g_outputStride;

template <class T>
inline void Release(T** p)
{
    if (p && *p)
    {
        (*p)->Release();
        *p = nullptr;
    }
}

D2D1_COLOR_F C(BYTE r, BYTE g, BYTE b, float a = 1.0f);
void Brush(BYTE r, BYTE g, BYTE b, float a = 1.0f);
bool Inside(float x, float y, const D2D1_RECT_F& r);

std::wstring Q(const std::wstring& s);
bool Exists(const std::wstring& p);
std::wstring AppFolder();
std::wstring Runtime();
std::wstring Python();
std::wstring Models();
void Refresh();
std::wstring LogPath();
std::wstring WorkerScript();

bool WriteUtf8File(const std::wstring& path, const std::string& text);
bool RunHiddenLogged(
    const std::wstring& exe,
    const std::wstring& args,
    const std::wstring& logPath,
    DWORD timeoutMs = INFINITE);
bool RunHidden(const std::wstring& exe, const std::wstring& args);
bool Download(const std::wstring& url, const std::wstring& path);

void WriteWorkerScript();
bool ValidateEngine();
bool PrepareRuntime();

HRESULT Graphics();
HRESULT LoadBitmap(const std::wstring& path, ID2D1Bitmap** out);
void Text(
    const wchar_t* t,
    const D2D1_RECT_F& r,
    BYTE red,
    BYTE green,
    BYTE blue,
    float a = 1.0f,
    bool center = false);

void Logo();
void ImageIcon(float cx, float cy);
void EraserIcon(float cx, float cy);
void SettingsIcon(float cx, float cy);
void DrawPerimeterRange(float w, float h, float start, float length, float stroke, float alpha);
void DrawAnimatedWindowBorder(float w, float h);
void SaveIcon(float cx, float cy);

D2D1_RECT_F CalcFitRect(ID2D1Bitmap* b, const D2D1_RECT_F& a);
D2D1_RECT_F DrawBitmapFit(ID2D1Bitmap* b, const D2D1_RECT_F& a, float opacity = 1.0f);

D2D1_RECT_F MainButton(float w);
D2D1_RECT_F ChangeButton();
D2D1_RECT_F SaveButton(float w);
D2D1_RECT_F EraserButtonRect(float w);
D2D1_RECT_F SettingsButtonRect(float w);
D2D1_RECT_F SettingsPopupRect(float w);
D2D1_RECT_F FeatherTrackRect(float w);
D2D1_RECT_F ChooseCardRect(float w);
D2D1_RECT_F ChooseSelectButtonRect(float w);
D2D1_RECT_F PreviewCardRect(float w);
D2D1_RECT_F CurrentPreviewImageRect(float w);

bool RebuildOutputBitmapFromPixels();
bool LoadEditableOutput(const std::wstring& path);
bool SaveEditedOutputPng(const std::wstring& path);
void UpdateFeatherFromX(float x, float width);
bool RefineSpotAtPoint(float x, float y, float width);

void Paint();
bool ChooseImage();
void Process();
void Save();

LRESULT Hit(HWND h, LPARAM lp);
LRESULT CALLBACK Proc(HWND h, UINT m, WPARAM w, LPARAM l);
