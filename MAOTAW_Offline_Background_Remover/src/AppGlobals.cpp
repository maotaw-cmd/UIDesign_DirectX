#include "App.h"

HWND g_hwnd = nullptr;

ID2D1Factory* g_d2d = nullptr;
ID2D1HwndRenderTarget* g_rt = nullptr;
ID2D1SolidColorBrush* g_brush = nullptr;
ID2D1GradientStopCollection* g_stops = nullptr;
ID2D1LinearGradientBrush* g_gradient = nullptr;

IDWriteFactory* g_dw = nullptr;
IDWriteTextFormat* g_font = nullptr;
IWICImagingFactory* g_wic = nullptr;

ID2D1Bitmap* g_inputBitmap = nullptr;
ID2D1Bitmap* g_outputBitmap = nullptr;

std::wstring g_inputPath;
std::wstring g_outputPath;
std::wstring g_status = L"Preparing offline background remover...";

std::atomic<bool> g_ready(false);
std::atomic<bool> g_busy(false);
std::atomic<float> g_progress(0.0f);
std::atomic<float> g_progressTarget(0.0f);
std::atomic<int> g_processPercent(0);

float g_mouseX = -1000.0f;
float g_mouseY = -1000.0f;
float g_resultAnim = 0.0f;
float g_borderPhase = 0.0f;

bool g_showSettings = false;
bool g_eraserMode = false;
bool g_dragSlider = false;
bool g_isErasing = false;
float g_featherValue = 0.55f;

std::vector<BYTE> g_outputPixels;
UINT g_outputPixelWidth = 0;
UINT g_outputPixelHeight = 0;
UINT g_outputStride = 0;
