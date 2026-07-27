// UPDATED VERSION: raw Direct2D pointer overloads for older Windows SDKs
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <wincodec.h>
#include <urlmon.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cwctype>
#include <set>

#include "../include/App.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

static const int WINDOW_WIDTH = AppConfig::WindowWidth;
static const int WINDOW_HEIGHT = AppConfig::WindowHeight;
static const float SIDEBAR_WIDTH = AppConfig::SidebarWidth;

static HWND g_window = NULL;
static ID2D1Factory* g_factory = NULL;
static ID2D1HwndRenderTarget* g_renderTarget = NULL;
static ID2D1SolidColorBrush* g_brush = NULL;
static ID2D1GradientStopCollection* g_stops = NULL;
static ID2D1LinearGradientBrush* g_gradient = NULL;
static IDWriteFactory* g_writeFactory = NULL;
static IDWriteTextFormat* g_textFormat = NULL;
static IWICImagingFactory* g_wicFactory = NULL;
static ID2D1Bitmap* g_previewBitmap = NULL;
static std::wstring g_previewPath;
static std::wstring g_statusText;
static std::wstring g_saveFolder;
static int g_deleteOlderDays = 0; // 0 = Never
static bool g_daysDropdownOpen = false;

static std::vector<LibraryItem> g_library;
static int g_activePage = 0; // 0 Home, 1 Library, 2 Favorites, 3 Settings
static bool g_inputFocused = false;
static bool g_caretVisible = true;
static std::wstring g_searchText;
static size_t g_inputCaret = 0;
static size_t g_inputAnchor = 0;
static size_t g_inputViewStart = 0;
static bool g_inputMouseSelecting = false;

// Forward declarations used by clipboard and keyboard handlers.
static void NormalizeInputSelection();
static bool HasInputSelection();
static void DeleteInputSelection();
static float g_mouseX = -1000.0f;
static float g_mouseY = -1000.0f;

// Small UI animations. Values move from 0.0f to 1.0f on a 16 ms timer.
static float g_previewAnimation = 0.0f;
static float g_previewTarget = 0.0f;
static float g_downloadPress = 0.0f;
static float g_savePress = 0.0f;
static bool g_clearPreviewAfterAnimation = false;
static int g_deletingLibraryIndex = -1;
static float g_libraryDeleteAnimation = 0.0f; // 0 = normal, 1 = fully removed
static int g_libraryPreviewIndex = -1; // full in-window preview
static float g_libraryScroll = 0.0f;
static float g_favoritesScroll = 0.0f;
static bool g_scrollbarDragging = false;
static float g_scrollbarDragOffset = 0.0f;

template <typename T>
static void SafeRelease(T** object)
{
    if (object != NULL && *object != NULL)
    {
        (*object)->Release();
        *object = NULL;
    }
}

static D2D1_COLOR_F MakeColor(BYTE r, BYTE g, BYTE b, float a = 1.0f)
{
    D2D1_COLOR_F color;
    color.r = static_cast<float>(r) / 255.0f;
    color.g = static_cast<float>(g) / 255.0f;
    color.b = static_cast<float>(b) / 255.0f;
    color.a = a;
    return color;
}

static void DiscardGraphics()
{
    SafeRelease(&g_gradient);
    SafeRelease(&g_stops);
    SafeRelease(&g_brush);
    SafeRelease(&g_previewBitmap);
    for (size_t i = 0; i < g_library.size(); ++i)
        SafeRelease(&g_library[i].bitmap);
    SafeRelease(&g_renderTarget);
}

static HRESULT CreateGraphics()
{
    if (g_renderTarget != NULL)
        return S_OK;

    RECT client = {};
    GetClientRect(g_window, &client);

    UINT width = static_cast<UINT>(client.right - client.left);
    UINT height = static_cast<UINT>(client.bottom - client.top);
    if (width == 0) width = 1;
    if (height == 0) height = 1;

    D2D1_RENDER_TARGET_PROPERTIES targetProperties = {};
    targetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    targetProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
    targetProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    targetProperties.dpiX = 0.0f;
    targetProperties.dpiY = 0.0f;
    targetProperties.usage = D2D1_RENDER_TARGET_USAGE_NONE;
    targetProperties.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

    D2D1_HWND_RENDER_TARGET_PROPERTIES windowProperties = {};
    windowProperties.hwnd = g_window;
    windowProperties.pixelSize.width = width;
    windowProperties.pixelSize.height = height;
    windowProperties.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;

    HRESULT hr = g_factory->CreateHwndRenderTarget(
        &targetProperties,
        &windowProperties,
        &g_renderTarget);

    if (FAILED(hr)) return hr;

    D2D1_COLOR_F whiteBrushColor = MakeColor(255, 255, 255);
    hr = g_renderTarget->CreateSolidColorBrush(
        &whiteBrushColor,
        NULL,
        &g_brush);

    if (FAILED(hr)) return hr;

    D2D1_GRADIENT_STOP gradientStops[3] = {};
    gradientStops[0].position = 0.0f;
    gradientStops[0].color = MakeColor(21, 25, 30);
    gradientStops[1].position = 0.45f;
    gradientStops[1].color = MakeColor(16, 20, 24);
    gradientStops[2].position = 1.0f;
    gradientStops[2].color = MakeColor(11, 14, 17);

    hr = g_renderTarget->CreateGradientStopCollection(
        gradientStops,
        3,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &g_stops);

    if (FAILED(hr)) return hr;

    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientProperties = {};
    gradientProperties.startPoint.x = 0.0f;
    gradientProperties.startPoint.y = 0.0f;
    gradientProperties.endPoint.x = static_cast<float>(width);
    gradientProperties.endPoint.y = static_cast<float>(height);

    hr = g_renderTarget->CreateLinearGradientBrush(
        &gradientProperties,
        NULL,
        g_stops,
        &g_gradient);

    return hr;
}


static bool IsYouTubeIdCharacter(wchar_t c)
{
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') ||
           (c >= L'0' && c <= L'9') || c == L'_' || c == L'-';
}

static std::wstring TrimInputText(std::wstring value)
{
    while (!value.empty() && (iswspace(value.front()) || value.front() == L'\"' || value.front() == L'\''))
        value.erase(value.begin());
    while (!value.empty() && (iswspace(value.back()) || value.back() == L'\"' || value.back() == L'\''))
        value.pop_back();
    return value;
}

static std::wstring ReadYouTubeIdAt(const std::wstring& value, size_t start)
{
    std::wstring id;
    while (start < value.size() && IsYouTubeIdCharacter(value[start]) && id.size() < 11)
        id.push_back(value[start++]);
    return id.size() == 11 ? id : L"";
}

static std::wstring ExtractYouTubeId(const std::wstring& rawInput)
{
    std::wstring value = TrimInputText(rawInput);
    if (value.empty()) return L"";

    // Accept a plain 11-character video ID too.
    if (value.size() == 11)
    {
        bool valid = true;
        for (size_t i = 0; i < value.size(); ++i)
            if (!IsYouTubeIdCharacter(value[i])) { valid = false; break; }
        if (valid) return value;
    }

    // Normal, shortened, Shorts, Live and Embed YouTube links.
    const wchar_t* markers[] = { L"youtu.be/", L"v=", L"shorts/", L"live/", L"embed/" };
    const size_t markerLengths[] = { 9, 2, 7, 5, 6 };
    for (int i = 0; i < 5; ++i)
    {
        size_t p = value.find(markers[i]);
        if (p != std::wstring::npos)
        {
            std::wstring id = ReadYouTubeIdAt(value, p + markerLengths[i]);
            if (!id.empty()) return id;
        }
    }

    return L"";
}

static HRESULT LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap);

static std::wstring PicturesFolderPath()
{
    wchar_t pictures[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, pictures)))
        lstrcpyW(pictures, L".");
    return pictures;
}

static std::wstring DefaultThumbnailFolderPath()
{
    std::wstring folder = PicturesFolderPath() + L"\\Maotaw Thumbnails";
    CreateDirectoryW(folder.c_str(), NULL);
    return folder;
}

static std::wstring TrimTrailingSlashes(std::wstring path)
{
    while (path.size() > 3 && !path.empty() && (path.back() == L'\\' || path.back() == L'/'))
        path.pop_back();
    return path;
}

static bool EndsWithFolderName(const std::wstring& path, const wchar_t* folderName)
{
    std::wstring clean = TrimTrailingSlashes(path);
    size_t slash = clean.find_last_of(L"\\/");
    std::wstring leaf = slash == std::wstring::npos ? clean : clean.substr(slash + 1);
    return _wcsicmp(leaf.c_str(), folderName) == 0;
}

static std::wstring MakeThumbnailFolderFromSelection(const std::wstring& selectedFolder)
{
    std::wstring base = TrimTrailingSlashes(selectedFolder);
    if (base.empty()) return DefaultThumbnailFolderPath();
    if (EndsWithFolderName(base, L"Maotaw Thumbnails")) return base;
    return base + L"\\Maotaw Thumbnails";
}

static bool IsPathInsideFolder(const std::wstring& filePath, const std::wstring& folderPath)
{
    std::wstring file = TrimTrailingSlashes(filePath);
    std::wstring folder = TrimTrailingSlashes(folderPath);
    if (file.size() <= folder.size()) return false;
    if (_wcsnicmp(file.c_str(), folder.c_str(), folder.size()) != 0) return false;
    wchar_t separator = file[folder.size()];
    return separator == L'\\' || separator == L'/';
}

static std::wstring ParentFolderPath(const std::wstring& path)
{
    std::wstring clean = TrimTrailingSlashes(path);
    size_t slash = clean.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return clean;
    return clean.substr(0, slash);
}

static std::wstring AppDataFolderPath()
{
    wchar_t folder[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, folder)))
        return PicturesFolderPath();
    std::wstring path = std::wstring(folder) + L"\\MaotawThumbnailDownloader";
    CreateDirectoryW(path.c_str(), NULL);
    return path;
}

static std::wstring LibraryStatePath()
{
    return AppDataFolderPath() + L"\\Maotaw_Thumbnail_Library.txt";
}

static std::wstring SettingsStatePath()
{
    return AppDataFolderPath() + L"\\settings.txt";
}

static bool EnsureFolderExists(const std::wstring& folder)
{
    if (folder.empty()) return false;
    DWORD attributes = GetFileAttributesW(folder.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES)
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    const int result = SHCreateDirectoryExW(NULL, folder.c_str(), NULL);
    return result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS ||
           GetFileAttributesW(folder.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static bool IsRegularFile(const std::wstring& path)
{
    DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static void SaveSettingsState()
{
    EnsureFolderExists(AppDataFolderPath());
    const std::wstring finalPath = SettingsStatePath();
    const std::wstring tempPath = finalPath + L".tmp";
    {
        std::wofstream file(tempPath.c_str(), std::ios::trunc);
        if (!file.is_open()) return;
        file << g_deleteOlderDays << L"\n" << g_saveFolder << L"\n";
        file.flush();
        if (!file.good()) return;
    }
    MoveFileExW(tempPath.c_str(), finalPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

static void LoadSettingsState()
{
    g_saveFolder = DefaultThumbnailFolderPath();
    std::wifstream file(SettingsStatePath().c_str());
    if (file.is_open())
    {
        std::wstring line;
        if (std::getline(file, line))
        {
            int days = _wtoi(line.c_str());
            if (days == 0 || days == 1 || days == 3 || days == 7 || days == 14 || days == 30 || days == 60 || days == 90)
                g_deleteOlderDays = days;
        }
        if (std::getline(file, line) && !line.empty())
            g_saveFolder = MakeThumbnailFolderFromSelection(line);
    }
    if (!EnsureFolderExists(g_saveFolder))
        g_saveFolder = DefaultThumbnailFolderPath();
    SaveSettingsState();
}

static void SaveLibraryState()
{
    EnsureFolderExists(AppDataFolderPath());
    const std::wstring finalPath = LibraryStatePath();
    const std::wstring tempPath = finalPath + L".tmp";
    {
        std::wofstream file(tempPath.c_str(), std::ios::trunc);
        if (!file.is_open()) return;
        for (size_t i = 0; i < g_library.size(); ++i)
        {
            if (!IsRegularFile(g_library[i].path)) continue;
            std::wstring cleanTitle = g_library[i].title;
            std::replace(cleanTitle.begin(), cleanTitle.end(), L'|', L'_');
            file << (g_library[i].favorite ? 1 : 0) << L"|" << cleanTitle << L"|" << g_library[i].path << L"\n";
        }
        file.flush();
        if (!file.good()) return;
    }
    MoveFileExW(tempPath.c_str(), finalPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

static void LoadLibraryState()
{
    for (size_t i = 0; i < g_library.size(); ++i)
        SafeRelease(&g_library[i].bitmap);
    g_library.clear();

    std::wifstream file(LibraryStatePath().c_str());
    if (!file.is_open()) return;
    std::wstring line;
    while (std::getline(file, line))
    {
        size_t p1 = line.find(L'|');
        size_t p2 = p1 == std::wstring::npos ? std::wstring::npos : line.find(L'|', p1 + 1);
        if (p1 == std::wstring::npos || p2 == std::wstring::npos) continue;
        LibraryItem item = {};
        item.favorite = line.substr(0, p1) == L"1";
        item.title = line.substr(p1 + 1, p2 - p1 - 1);
        item.path = line.substr(p2 + 1);
        item.bitmap = NULL;
        if (IsRegularFile(item.path) && IsPathInsideFolder(item.path, g_saveFolder))
            g_library.push_back(item);
    }
}

static void RecoverLibraryFromSaveFolder()
{
    if (!EnsureFolderExists(g_saveFolder)) return;
    std::set<std::wstring> known;
    for (size_t i = 0; i < g_library.size(); ++i) known.insert(g_library[i].path);

    std::wstring prefix = g_saveFolder;
    if (!prefix.empty() && prefix.back() != L'\\' && prefix.back() != L'/') prefix += L"\\";
    WIN32_FIND_DATAW data = {};
    HANDLE handle = FindFirstFileW((prefix + L"YouTube_Thumbnail_*.jpg").c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE) return;
    do
    {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
        std::wstring fullPath = prefix + data.cFileName;
        if (known.find(fullPath) != known.end()) continue;
        LibraryItem item = {};
        item.path = fullPath;
        item.title = data.cFileName;
        item.favorite = false;
        item.bitmap = NULL;
        g_library.push_back(item);
        known.insert(fullPath);
    } while (FindNextFileW(handle, &data));
    FindClose(handle);
    SaveLibraryState();
}

static void EnsureLibraryBitmaps()
{
    if (g_renderTarget == NULL) return;
    for (size_t i = 0; i < g_library.size(); ++i)
        if (g_library[i].bitmap == NULL)
            LoadBitmapFromFile(g_library[i].path, &g_library[i].bitmap);
}

static std::wstring TempThumbnailPath()
{
    // Use a new temporary file for every request. Reusing one fixed file can
    // leave the next download stuck if Windows, WIC, antivirus, or the URL
    // cache still has the previous file open for a short time.
    wchar_t folder[MAX_PATH] = {};
    if (GetTempPathW(MAX_PATH, folder) == 0)
        return L"maotaw_thumbnail_temp.jpg";

    wchar_t tempFile[MAX_PATH] = {};
    if (GetTempFileNameW(folder, L"MTW", 0, tempFile) == 0)
    {
        SYSTEMTIME st = {};
        GetLocalTime(&st);
        wchar_t fallback[MAX_PATH] = {};
        wsprintfW(fallback, L"%smaotaw_thumbnail_%04d%02d%02d_%02d%02d%02d_%03d.jpg",
            folder, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
            st.wSecond, st.wMilliseconds);
        return fallback;
    }

    DeleteFileW(tempFile); // URLDownloadToFileW creates the actual image file.
    std::wstring path = tempFile;
    const size_t dot = path.find_last_of(L'.');
    if (dot != std::wstring::npos) path.erase(dot);
    path += L".jpg";
    return path;
}

static HRESULT LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap)
{
    if (bitmap == NULL || g_wicFactory == NULL || g_renderTarget == NULL) return E_FAIL;
    *bitmap = NULL;

    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* frame = NULL;
    IWICFormatConverter* converter = NULL;

    HRESULT hr = g_wicFactory->CreateDecoderFromFilename(path.c_str(), NULL, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &decoder);
    if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, &frame);
    if (SUCCEEDED(hr)) hr = g_wicFactory->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr)) hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);
    if (SUCCEEDED(hr)) hr = g_renderTarget->CreateBitmapFromWicBitmap(converter, NULL, bitmap);

    SafeRelease(&converter);
    SafeRelease(&frame);
    SafeRelease(&decoder);
    return hr;
}

static bool FetchPreview()
{
    // Work from a stable copy so an earlier failed attempt cannot corrupt the next one.
    const std::wstring submittedLink = TrimInputText(g_searchText);
    std::wstring id = ExtractYouTubeId(submittedLink);
    if (id.empty())
    {
        g_statusText = L"Paste a valid YouTube link.";
        g_inputFocused = true;
        g_inputAnchor = 0;
        g_inputCaret = g_searchText.size();
        g_inputViewStart = 0;
        g_caretVisible = true;
        SetFocus(g_window);
        return false;
    }

    // Fully release and remove any previous temporary preview before starting.
    SafeRelease(&g_previewBitmap);
    if (!g_previewPath.empty()) DeleteFileW(g_previewPath.c_str());
    g_previewPath.clear();

    std::wstring url = L"https://img.youtube.com/vi/" + id + L"/maxresdefault.jpg";
    std::wstring path = TempThumbnailPath();
    DeleteFileW(path.c_str());

    HRESULT hr = URLDownloadToFileW(NULL, url.c_str(), path.c_str(), 0, NULL);
    if (FAILED(hr))
    {
        url = L"https://img.youtube.com/vi/" + id + L"/hqdefault.jpg";
        hr = URLDownloadToFileW(NULL, url.c_str(), path.c_str(), 0, NULL);
    }

    if (FAILED(hr))
    {
        DeleteFileW(path.c_str());
        g_previewPath.clear();
        SafeRelease(&g_previewBitmap);
        g_statusText = L"Could not download the thumbnail. Try again.";
        g_inputFocused = true;
        g_inputAnchor = 0;
        g_inputCaret = g_searchText.size();
        g_inputViewStart = 0;
        SetFocus(g_window);
        return false;
    }

    SafeRelease(&g_previewBitmap);
    if (FAILED(LoadBitmapFromFile(path, &g_previewBitmap)))
    {
        DeleteFileW(path.c_str());
        g_previewPath.clear();
        SafeRelease(&g_previewBitmap);
        g_statusText = L"The thumbnail could not be opened. Try again.";
        g_inputFocused = true;
        g_inputAnchor = 0;
        g_inputCaret = g_searchText.size();
        g_inputViewStart = 0;
        SetFocus(g_window);
        return false;
    }

    g_previewPath = path;
    g_statusText = L"Thumbnail ready.";
    return true;
}

static bool SavePreviewToLibrary()
{
    if (g_previewBitmap == NULL || g_previewPath.empty())
    {
        g_statusText = L"Load a thumbnail first.";
        return false;
    }

    std::wstring pictures = g_saveFolder.empty() ? DefaultThumbnailFolderPath() : g_saveFolder;
    if (!EnsureFolderExists(pictures))
    {
        g_statusText = L"The selected save folder is unavailable.";
        return false;
    }

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    wchar_t name[128] = {};
    wsprintfW(name, L"YouTube_Thumbnail_%04d%02d%02d_%02d%02d%02d_%03d.jpg",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    std::wstring destination = pictures;
    if (!destination.empty() && destination.back() != L'\\' && destination.back() != L'/') destination += L"\\";
    destination += name;

    if (!CopyFileW(g_previewPath.c_str(), destination.c_str(), FALSE))
    {
        g_statusText = L"Could not save the thumbnail.";
        return false;
    }

    LibraryItem item = {};
    item.path = destination;
    item.title = name;
    item.favorite = false;
    item.bitmap = NULL;
    LoadBitmapFromFile(destination, &item.bitmap);
    g_library.push_back(item);
    SaveLibraryState();
    g_statusText = L"Saved in Maotaw Thumbnails and added to Library.";
    return true;
}

static bool PasteClipboardText()
{
    if (!OpenClipboard(g_window)) return false;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data == NULL)
    {
        CloseClipboard();
        return false;
    }

    const wchar_t* text = static_cast<const wchar_t*>(GlobalLock(data));
    if (text != NULL)
    {
        DeleteInputSelection();
        std::wstring pasted(text);
        if (g_searchText.size() + pasted.size() > 180)
            pasted.resize(180 - g_searchText.size());
        g_searchText.insert(g_inputCaret, pasted);
        g_inputCaret += pasted.size();
        g_inputAnchor = g_inputCaret;
        NormalizeInputSelection();
        GlobalUnlock(data);
    }
    CloseClipboard();
    return text != NULL;
}

static bool CopyInputToClipboard(bool clearAfterCopy)
{
    std::wstring copied;
    if (HasInputSelection())
    {
        const size_t a = (std::min)(g_inputCaret, g_inputAnchor);
        const size_t b = (std::max)(g_inputCaret, g_inputAnchor);
        copied = g_searchText.substr(a, b - a);
    }
    else copied = g_searchText;

    if (!OpenClipboard(g_window)) return false;
    EmptyClipboard();
    const SIZE_T bytes = (copied.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (memory == NULL) { CloseClipboard(); return false; }
    void* target = GlobalLock(memory);
    if (target == NULL) { GlobalFree(memory); CloseClipboard(); return false; }
    CopyMemory(target, copied.c_str(), bytes);
    GlobalUnlock(memory);
    SetClipboardData(CF_UNICODETEXT, memory);
    CloseClipboard();

    if (clearAfterCopy)
    {
        if (HasInputSelection()) DeleteInputSelection();
        else
        {
            g_searchText.clear();
            g_inputCaret = g_inputAnchor = g_inputViewStart = 0;
        }
    }
    return true;
}

static void DrawLogo()
{
    ID2D1PathGeometry* geometry = NULL;
    ID2D1GeometrySink* sink = NULL;

    if (FAILED(g_factory->CreatePathGeometry(&geometry)))
        return;

    if (FAILED(geometry->Open(&sink)))
    {
        geometry->Release();
        return;
    }

    D2D1_POINT_2F top = { 39.0f, 21.0f };
    D2D1_POINT_2F left = { 26.0f, 45.0f };
    D2D1_POINT_2F right = { 52.0f, 45.0f };

    sink->BeginFigure(top, D2D1_FIGURE_BEGIN_HOLLOW);
    sink->AddLine(left);
    sink->AddLine(right);
    sink->AddLine(top);
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
    sink->Release();

    D2D1_COLOR_F logoColor = MakeColor(242, 244, 246);
    g_brush->SetColor(&logoColor);
    g_renderTarget->DrawGeometry(geometry, g_brush, 3.0f, NULL);
    geometry->Release();
}


