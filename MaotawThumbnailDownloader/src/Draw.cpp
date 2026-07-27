#include "../include/Draw.h"

static void SetBrushColor(BYTE r, BYTE g, BYTE b, float a = 1.0f)
{
    D2D1_COLOR_F color = MakeColor(r, g, b, a);
    g_brush->SetColor(&color);
}

static void DrawLineAA(float x1, float y1, float x2, float y2, float width)
{
    D2D1_POINT_2F a = { x1, y1 };
    D2D1_POINT_2F b = { x2, y2 };
    g_renderTarget->DrawLine(a, b, g_brush, width, NULL);
}

static void DrawHomeIcon(float cx, float cy, bool selected)
{
    SetBrushColor(selected ? 242 : 174, selected ? 244 : 181, selected ? 246 : 188, selected ? 1.0f : 0.92f);

    ID2D1PathGeometry* geometry = NULL;
    ID2D1GeometrySink* sink = NULL;
    if (FAILED(g_factory->CreatePathGeometry(&geometry))) return;
    if (FAILED(geometry->Open(&sink))) { geometry->Release(); return; }

    D2D1_POINT_2F start = { cx - 9.0f, cy - 1.0f };
    sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);
    sink->AddLine({ cx, cy - 9.0f });
    sink->AddLine({ cx + 9.0f, cy - 1.0f });
    sink->AddLine({ cx + 7.0f, cy - 1.0f });
    sink->AddLine({ cx + 7.0f, cy + 8.0f });
    sink->AddLine({ cx + 2.5f, cy + 8.0f });
    sink->AddLine({ cx + 2.5f, cy + 2.5f });
    sink->AddLine({ cx - 2.5f, cy + 2.5f });
    sink->AddLine({ cx - 2.5f, cy + 8.0f });
    sink->AddLine({ cx - 7.0f, cy + 8.0f });
    sink->AddLine({ cx - 7.0f, cy - 1.0f });
    sink->AddLine(start);
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
    sink->Release();

    g_renderTarget->DrawGeometry(geometry, g_brush, 1.65f, NULL);
    geometry->Release();
}

static void DrawLibraryIcon(float cx, float cy, bool selected)
{
    SetBrushColor(selected ? 242 : 174, selected ? 244 : 181, selected ? 246 : 188, selected ? 1.0f : 0.92f);

    D2D1_RECT_F first  = { cx - 9.0f, cy - 8.0f, cx - 4.5f, cy + 7.0f };
    D2D1_RECT_F second = { cx - 2.2f, cy - 9.0f, cx + 2.2f, cy + 7.0f };
    D2D1_RECT_F third  = { cx + 4.5f, cy - 6.5f, cx + 9.0f, cy + 7.0f };

    g_renderTarget->DrawRectangle(&first, g_brush, 1.45f, NULL);
    g_renderTarget->DrawRectangle(&second, g_brush, 1.45f, NULL);
    g_renderTarget->DrawRectangle(&third, g_brush, 1.45f, NULL);

    DrawLineAA(cx - 7.8f, cy - 5.0f, cx - 5.7f, cy - 5.0f, 1.15f);
    DrawLineAA(cx - 1.0f, cy - 6.0f, cx + 1.0f, cy - 6.0f, 1.15f);
    DrawLineAA(cx + 5.7f, cy - 3.5f, cx + 7.8f, cy - 3.5f, 1.15f);
    DrawLineAA(cx - 10.5f, cy + 9.5f, cx + 10.5f, cy + 9.5f, 1.45f);
}

static void DrawFavoritesIcon(float cx, float cy, bool selected)
{
    SetBrushColor(selected ? 242 : 174, selected ? 244 : 181, selected ? 246 : 188, selected ? 1.0f : 0.92f);

    ID2D1PathGeometry* geometry = NULL;
    ID2D1GeometrySink* sink = NULL;
    if (FAILED(g_factory->CreatePathGeometry(&geometry))) return;
    if (FAILED(geometry->Open(&sink))) { geometry->Release(); return; }

    D2D1_POINT_2F bottom = { cx, cy + 9.0f };
    sink->BeginFigure(bottom, D2D1_FIGURE_BEGIN_HOLLOW);

    D2D1_BEZIER_SEGMENT leftA = {};
    leftA.point1 = { cx - 2.0f, cy + 6.8f };
    leftA.point2 = { cx - 9.0f, cy + 2.5f };
    leftA.point3 = { cx - 9.0f, cy - 3.0f };
    sink->AddBezier(leftA);

    D2D1_BEZIER_SEGMENT leftB = {};
    leftB.point1 = { cx - 9.0f, cy - 8.0f };
    leftB.point2 = { cx - 2.8f, cy - 9.5f };
    leftB.point3 = { cx, cy - 4.8f };
    sink->AddBezier(leftB);

    D2D1_BEZIER_SEGMENT rightA = {};
    rightA.point1 = { cx + 2.8f, cy - 9.5f };
    rightA.point2 = { cx + 9.0f, cy - 8.0f };
    rightA.point3 = { cx + 9.0f, cy - 3.0f };
    sink->AddBezier(rightA);

    D2D1_BEZIER_SEGMENT rightB = {};
    rightB.point1 = { cx + 9.0f, cy + 2.5f };
    rightB.point2 = { cx + 2.0f, cy + 6.8f };
    rightB.point3 = bottom;
    sink->AddBezier(rightB);

    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    g_renderTarget->DrawGeometry(geometry, g_brush, 1.55f, NULL);
    geometry->Release();
}

static void DrawSettingsIcon(float cx, float cy, bool selected)
{
    SetBrushColor(selected ? 242 : 174, selected ? 244 : 181, selected ? 246 : 188, selected ? 1.0f : 0.92f);

    D2D1_ELLIPSE ring = { { cx, cy }, 7.3f, 7.3f };
    D2D1_ELLIPSE hub  = { { cx, cy }, 2.6f, 2.6f };
    g_renderTarget->DrawEllipse(&ring, g_brush, 1.5f, NULL);
    g_renderTarget->DrawEllipse(&hub, g_brush, 1.5f, NULL);

    const float k = 0.70710678f;
    const float directions[8][2] = {
        { 0.0f, -1.0f }, { k, -k }, { 1.0f, 0.0f }, { k, k },
        { 0.0f, 1.0f }, { -k, k }, { -1.0f, 0.0f }, { -k, -k }
    };

    for (int i = 0; i < 8; ++i)
    {
        float dx = directions[i][0];
        float dy = directions[i][1];
        DrawLineAA(cx + dx * 8.2f, cy + dy * 8.2f,
                   cx + dx * 10.8f, cy + dy * 10.8f, 1.75f);
    }
}

static float PageCenterY(int page, float height)
{
    if (page == 0) return 94.0f;
    if (page == 1) return 145.0f;
    if (page == 2) return 196.0f;
    return height - 37.0f;
}

static void DrawSidebarIcons(float height)
{
    const float cx = SIDEBAR_WIDTH * 0.5f;
    const float activeY = PageCenterY(g_activePage, height);

    D2D1_ROUNDED_RECT indicator = {};
    indicator.rect.left = 5.0f;
    indicator.rect.top = activeY - 6.0f;
    indicator.rect.right = 7.0f;
    indicator.rect.bottom = activeY + 6.0f;
    indicator.radiusX = 1.0f;
    indicator.radiusY = 1.0f;
    SetBrushColor(238, 241, 244, 0.96f);
    g_renderTarget->FillRoundedRectangle(&indicator, g_brush);

    DrawHomeIcon(cx, 94.0f, g_activePage == 0);
    DrawLibraryIcon(cx, 145.0f, g_activePage == 1);
    DrawFavoritesIcon(cx, 196.0f, g_activePage == 2);
    DrawSettingsIcon(cx, height - 37.0f, g_activePage == 3);
}

static int HitTestNavigation(float x, float y, float height)
{
    if (x < 10.0f || x > SIDEBAR_WIDTH - 8.0f)
        return -1;

    for (int page = 0; page < 4; ++page)
    {
        float cy = PageCenterY(page, height);
        if (y >= cy - 20.0f && y <= cy + 20.0f)
            return page;
    }
    return -1;
}

static D2D1_RECT_F GetSearchInputRect(float width)
{
    const float top = 24.0f;
    const float inputWidth = 250.0f;
    const float buttonWidth = 106.0f;
    const float gap = 8.0f;
    const float groupWidth = inputWidth + gap + buttonWidth;
    const float contentWidth = width - SIDEBAR_WIDTH;
    const float left = SIDEBAR_WIDTH + (contentWidth - groupWidth) * 0.5f;
    return { left, top, left + inputWidth, top + 34.0f };
}

static D2D1_RECT_F GetDownloadButtonRect(float width)
{
    D2D1_RECT_F input = GetSearchInputRect(width);
    return { input.right + 8.0f, input.top, input.right + 114.0f, input.bottom };
}

static bool PointInRectF(float x, float y, const D2D1_RECT_F& r)
{
    return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

static float Clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static float SmoothStep(float value)
{
    value = Clamp01(value);
    return value * value * (3.0f - 2.0f * value);
}

static D2D1_RECT_F ScaleRectFromCenter(const D2D1_RECT_F& rect, float scale)
{
    const float cx = (rect.left + rect.right) * 0.5f;
    const float cy = (rect.top + rect.bottom) * 0.5f;
    const float halfWidth = (rect.right - rect.left) * 0.5f * scale;
    const float halfHeight = (rect.bottom - rect.top) * 0.5f * scale;
    return { cx - halfWidth, cy - halfHeight, cx + halfWidth, cy + halfHeight };
}

static float MeasureSingleLineTextWidth(const std::wstring& text);

static const size_t INPUT_VISIBLE_CHARACTERS = 28;

static void NormalizeInputSelection()
{
    if (g_inputCaret > g_searchText.size()) g_inputCaret = g_searchText.size();
    if (g_inputAnchor > g_searchText.size()) g_inputAnchor = g_searchText.size();

    if (g_inputCaret < g_inputViewStart)
        g_inputViewStart = g_inputCaret;
    else if (g_inputCaret > g_inputViewStart + INPUT_VISIBLE_CHARACTERS)
        g_inputViewStart = g_inputCaret - INPUT_VISIBLE_CHARACTERS;

    const size_t maxStart = g_searchText.size() > INPUT_VISIBLE_CHARACTERS
        ? g_searchText.size() - INPUT_VISIBLE_CHARACTERS : 0;
    if (g_inputViewStart > maxStart) g_inputViewStart = maxStart;
}

static std::wstring VisibleInputText(const std::wstring& text)
{
    if (g_inputViewStart > text.size()) g_inputViewStart = text.size();
    return text.substr(g_inputViewStart, INPUT_VISIBLE_CHARACTERS);
}

static bool HasInputSelection()
{
    return g_inputCaret != g_inputAnchor;
}

static void DeleteInputSelection()
{
    if (!HasInputSelection()) return;
    const size_t a = (std::min)(g_inputCaret, g_inputAnchor);
    const size_t b = (std::max)(g_inputCaret, g_inputAnchor);
    g_searchText.erase(a, b - a);
    g_inputCaret = g_inputAnchor = a;
    NormalizeInputSelection();
}

static size_t InputIndexFromMouseX(float mouseX, const D2D1_RECT_F& input)
{
    const std::wstring visible = VisibleInputText(g_searchText);
    const float localX = mouseX - (input.left + 32.0f);
    if (localX <= 0.0f) return g_inputViewStart;

    for (size_t i = 0; i <= visible.size(); ++i)
    {
        const float w = MeasureSingleLineTextWidth(visible.substr(0, i));
        if (localX <= w) return g_inputViewStart + i;
    }
    return (std::min)(g_inputViewStart + visible.size(), g_searchText.size());
}


static float MeasureSingleLineTextWidth(const std::wstring& text)
{
    if (text.empty() || g_writeFactory == NULL || g_textFormat == NULL) return 0.0f;

    IDWriteTextLayout* layout = NULL;
    HRESULT hr = g_writeFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.size()),
        g_textFormat,
        4096.0f,
        64.0f,
        &layout);

    if (FAILED(hr) || layout == NULL) return 0.0f;

    DWRITE_TEXT_METRICS metrics = {};
    layout->GetMetrics(&metrics);
    layout->Release();
    return metrics.widthIncludingTrailingWhitespace;
}

static void DrawMagnifier(float cx, float cy)
{
    D2D1_ELLIPSE lens = { { cx - 2.0f, cy - 2.0f }, 4.8f, 4.8f };
    g_renderTarget->DrawEllipse(&lens, g_brush, 1.45f, NULL);
    DrawLineAA(cx + 1.6f, cy + 1.6f, cx + 6.0f, cy + 6.0f, 1.45f);
}

static void DrawDownloadIcon(float cx, float cy)
{
    DrawLineAA(cx, cy - 7.0f, cx, cy + 3.0f, 1.55f);
    DrawLineAA(cx - 4.0f, cy - 0.5f, cx, cy + 3.5f, 1.55f);
    DrawLineAA(cx + 4.0f, cy - 0.5f, cx, cy + 3.5f, 1.55f);
    DrawLineAA(cx - 6.0f, cy + 7.0f, cx + 6.0f, cy + 7.0f, 1.55f);
}

static void DrawTextSimple(const wchar_t* text, const D2D1_RECT_F& rect, BYTE r, BYTE g, BYTE b, float alpha = 1.0f)
{
    if (g_textFormat == NULL || text == NULL) return;
    SetBrushColor(r, g, b, alpha);
    g_renderTarget->DrawTextW(text, static_cast<UINT32>(wcslen(text)), g_textFormat, &rect, g_brush,
                             D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
}

static void DrawCenteredText(const wchar_t* text, const D2D1_RECT_F& rect, BYTE r, BYTE g, BYTE b, float alpha = 1.0f)
{
    if (g_textFormat == NULL || text == NULL) return;
    g_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    g_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    DrawTextSimple(text, rect, r, g, b, alpha);
    g_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    g_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

static D2D1_RECT_F GetPreviewRect(float width)
{
    // Bigger, more centered, and slightly lower on the page.
    const float w = 560.0f;
    const float h = 315.0f;
    const float top = 112.0f;
    const float contentWidth = width - SIDEBAR_WIDTH;
    const float left = SIDEBAR_WIDTH + (contentWidth - w) * 0.5f;
    return { left, top, left + w, top + h };
}

static D2D1_RECT_F GetSaveButtonRect(float width)
{
    D2D1_RECT_F preview = GetPreviewRect(width);
    const float saveWidth = 166.0f;
    const float closeWidth = 116.0f;
    const float gap = 10.0f;
    const float groupWidth = saveWidth + gap + closeWidth;
    const float left = preview.left + ((preview.right - preview.left) - groupWidth) * 0.5f;
    return { left, preview.bottom + 18.0f, left + saveWidth, preview.bottom + 54.0f };
}

static D2D1_RECT_F GetPreviewCloseRect(float width)
{
    D2D1_RECT_F save = GetSaveButtonRect(width);
    const float gap = 10.0f;
    const float closeWidth = 116.0f;
    return { save.right + gap, save.top, save.right + gap + closeWidth, save.bottom };
}

static void DrawHeartIcon(float cx, float cy, bool filled)
{
    ID2D1PathGeometry* geometry = NULL;
    ID2D1GeometrySink* sink = NULL;
    if (FAILED(g_factory->CreatePathGeometry(&geometry))) return;
    if (FAILED(geometry->Open(&sink))) { geometry->Release(); return; }
    D2D1_POINT_2F bottom = { cx, cy + 8.0f };
    sink->BeginFigure(bottom, filled ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
    D2D1_BEZIER_SEGMENT a = { {cx-3,cy+5},{cx-9,cy+1},{cx-8,cy-4} }; sink->AddBezier(a);
    D2D1_BEZIER_SEGMENT b = { {cx-7,cy-9},{cx-2,cy-9},{cx,cy-5} }; sink->AddBezier(b);
    D2D1_BEZIER_SEGMENT c = { {cx+2,cy-9},{cx+7,cy-9},{cx+8,cy-4} }; sink->AddBezier(c);
    D2D1_BEZIER_SEGMENT d = { {cx+9,cy+1},{cx+3,cy+5},bottom }; sink->AddBezier(d);
    sink->EndFigure(D2D1_FIGURE_END_CLOSED); sink->Close(); sink->Release();
    if (filled) g_renderTarget->FillGeometry(geometry, g_brush, NULL);
    else g_renderTarget->DrawGeometry(geometry, g_brush, 1.5f, NULL);
    geometry->Release();
}

static void DrawTrashIcon(float cx, float cy)
{
    D2D1_RECT_F bin = { cx - 5.5f, cy - 4.0f, cx + 5.5f, cy + 7.0f };
    g_renderTarget->DrawRectangle(&bin, g_brush, 1.4f, NULL);
    DrawLineAA(cx - 7.0f, cy - 7.0f, cx + 7.0f, cy - 7.0f, 1.4f);
    DrawLineAA(cx - 2.5f, cy - 9.0f, cx + 2.5f, cy - 9.0f, 1.4f);
}

static void DrawFolderIcon(float cx, float cy)
{
    // Clean outline folder icon.
    D2D1_POINT_2F points[] =
    {
        {cx - 9.0f, cy - 6.0f},
        {cx - 2.0f, cy - 6.0f},
        {cx + 1.0f, cy - 3.0f},
        {cx + 9.0f, cy - 3.0f},
        {cx + 9.0f, cy + 7.0f},
        {cx - 9.0f, cy + 7.0f}
    };

    ID2D1PathGeometry* geometry = NULL;
    ID2D1GeometrySink* sink = NULL;
    if (SUCCEEDED(g_factory->CreatePathGeometry(&geometry)) &&
        SUCCEEDED(geometry->Open(&sink)))
    {
        sink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_HOLLOW);
        for (int i = 1; i < 6; ++i) sink->AddLine(points[i]);
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        g_renderTarget->DrawGeometry(geometry, g_brush, 1.5f, NULL);
    }
    SafeRelease(&sink);
    SafeRelease(&geometry);
}

static std::wstring ShortLibraryTitle(const std::wstring& title)
{
    const size_t maxCharacters = 25;
    if (title.size() <= maxCharacters) return title;
    return title.substr(0, maxCharacters) + L"...";
}

static void OpenContainingFolder(const std::wstring& filePath)
{
    size_t slash = filePath.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return;
    std::wstring folder = filePath.substr(0, slash);
    ShellExecuteW(NULL, L"open", folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static bool ChooseSaveFolder(HWND owner)
{
    IFileDialog* dialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&dialog));
    if (FAILED(hr) || dialog == NULL) return false;

    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
    dialog->SetTitle(L"Choose where thumbnails are saved");

    if (!g_saveFolder.empty())
    {
        IShellItem* currentFolder = NULL;
        if (SUCCEEDED(SHCreateItemFromParsingName(ParentFolderPath(g_saveFolder).c_str(), NULL, IID_PPV_ARGS(&currentFolder))))
        {
            dialog->SetFolder(currentFolder);
            currentFolder->Release();
        }
    }

    hr = dialog->Show(owner);
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        dialog->Release();
        return false;
    }
    if (FAILED(hr))
    {
        dialog->Release();
        return false;
    }

    IShellItem* result = NULL;
    hr = dialog->GetResult(&result);
    if (FAILED(hr) || result == NULL)
    {
        dialog->Release();
        return false;
    }

    PWSTR selectedPath = NULL;
    hr = result->GetDisplayName(SIGDN_FILESYSPATH, &selectedPath);
    if (SUCCEEDED(hr) && selectedPath != NULL)
    {
        g_saveFolder = MakeThumbnailFolderFromSelection(selectedPath);
        EnsureFolderExists(g_saveFolder);
        SaveSettingsState();
        LoadLibraryState();
        RecoverLibraryFromSaveFolder();
        SaveLibraryState();
        CoTaskMemFree(selectedPath);
    }

    result->Release();
    dialog->Release();
    return SUCCEEDED(hr);
}

static void DeleteOldThumbnailsNow()
{
    if (g_deleteOlderDays <= 0) return;
    FILETIME nowFileTime = {};
    GetSystemTimeAsFileTime(&nowFileTime);
    ULARGE_INTEGER now = {}; now.LowPart = nowFileTime.dwLowDateTime; now.HighPart = nowFileTime.dwHighDateTime;
    const ULONGLONG maxAge = static_cast<ULONGLONG>(g_deleteOlderDays) * 24ULL * 60ULL * 60ULL * 10000000ULL;
    for (int i = static_cast<int>(g_library.size()) - 1; i >= 0; --i)
    {
        WIN32_FILE_ATTRIBUTE_DATA data = {};
        if (!GetFileAttributesExW(g_library[i].path.c_str(), GetFileExInfoStandard, &data)) continue;
        ULARGE_INTEGER written = {}; written.LowPart = data.ftLastWriteTime.dwLowDateTime; written.HighPart = data.ftLastWriteTime.dwHighDateTime;
        if (now.QuadPart > written.QuadPart && now.QuadPart - written.QuadPart > maxAge)
        {
            DeleteFileW(g_library[i].path.c_str());
            SafeRelease(&g_library[i].bitmap);
            g_library.erase(g_library.begin() + i);
        }
    }
    SaveLibraryState();
}

static void DrawHomePage(float width, float height)
{
    (void)height;
    D2D1_RECT_F input = GetSearchInputRect(width);
    D2D1_RECT_F topButton = GetDownloadButtonRect(width);

    D2D1_ROUNDED_RECT inputShape = { input, 4.0f, 4.0f };
    SetBrushColor(18, 22, 27, 0.94f); g_renderTarget->FillRoundedRectangle(&inputShape, g_brush);
    SetBrushColor(g_inputFocused ? 150 : 73, g_inputFocused ? 157 : 80, g_inputFocused ? 164 : 87, 0.92f);
    g_renderTarget->DrawRoundedRectangle(&inputShape, g_brush, 1.0f, NULL);
    SetBrushColor(153, 159, 166, 0.95f); DrawMagnifier(input.left + 17.0f, input.top + 16.5f);

    D2D1_RECT_F textRect = { input.left + 32.0f, input.top + 7.0f, input.right - 10.0f, input.bottom - 4.0f };
    g_textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    if (g_searchText.empty())
    {
        DrawTextSimple(L"Paste YouTube link...", textRect, 139, 145, 151, 0.80f);
    }
    else
    {
        const std::wstring visibleText = VisibleInputText(g_searchText);
        if (HasInputSelection())
        {
            const size_t selA = (std::min)(g_inputCaret, g_inputAnchor);
            const size_t selB = (std::max)(g_inputCaret, g_inputAnchor);
            const size_t visA = (std::max)(selA, g_inputViewStart);
            const size_t visB = (std::min)(selB, g_inputViewStart + visibleText.size());
            if (visB > visA)
            {
                const float x1 = textRect.left + MeasureSingleLineTextWidth(g_searchText.substr(g_inputViewStart, visA - g_inputViewStart));
                const float x2 = textRect.left + MeasureSingleLineTextWidth(g_searchText.substr(g_inputViewStart, visB - g_inputViewStart));
                D2D1_RECT_F selectionRect = { x1, input.top + 5.0f, x2, input.bottom - 5.0f };
                SetBrushColor(83, 119, 166, 0.65f);
                g_renderTarget->FillRectangle(&selectionRect, g_brush);
            }
        }
        DrawTextSimple(visibleText.c_str(), textRect, 235, 238, 241, 1.0f);
    }

    if (g_inputFocused && g_caretVisible)
    {
        NormalizeInputSelection();
        const size_t relativeCaret = g_inputCaret >= g_inputViewStart ? g_inputCaret - g_inputViewStart : 0;
        const std::wstring beforeCaret = g_searchText.substr(g_inputViewStart, relativeCaret);
        float caretX = input.left + 32.0f + MeasureSingleLineTextWidth(beforeCaret) + 1.0f;
        if (caretX > input.right - 11.0f) caretX = input.right - 11.0f;
        SetBrushColor(239, 242, 245, 1.0f);
        DrawLineAA(caretX, input.top + 8.0f, caretX, input.bottom - 8.0f, 1.0f);
    }

    const float downloadScale = 1.0f - 0.035f * SmoothStep(g_downloadPress);
    D2D1_RECT_F animatedTopButton = ScaleRectFromCenter(topButton, downloadScale);
    D2D1_ROUNDED_RECT topShape = { animatedTopButton, 4.0f, 4.0f };
    const bool hoverTopDownload = PointInRectF(g_mouseX, g_mouseY, topButton);
    SetBrushColor(hoverTopDownload ? 255 : 235, hoverTopDownload ? 255 : 238, hoverTopDownload ? 255 : 241, 1.0f);
    g_renderTarget->FillRoundedRectangle(&topShape, g_brush);
    SetBrushColor(25, 29, 34, 1.0f);
    DrawDownloadIcon(animatedTopButton.left + 17.0f, animatedTopButton.top + 16.0f);
    DrawCenteredText(L"Download", {animatedTopButton.left+25.0f,animatedTopButton.top,animatedTopButton.right-4.0f,animatedTopButton.bottom}, 25,29,34,1.0f);

    // The preview softly scales and fades in after Download. It scales out after Save.
    if (g_previewBitmap != NULL && g_previewAnimation > 0.001f)
    {
        const float eased = SmoothStep(g_previewAnimation);
        D2D1_RECT_F previewBase = GetPreviewRect(width);
        D2D1_RECT_F preview = ScaleRectFromCenter(previewBase, 0.92f + 0.08f * eased);
        D2D1_RECT_F saveButtonBase = GetSaveButtonRect(width);
        const float saveOffset = (1.0f - eased) * 12.0f;
        D2D1_RECT_F saveButton = { saveButtonBase.left, saveButtonBase.top + saveOffset, saveButtonBase.right, saveButtonBase.bottom + saveOffset };

        D2D1_ROUNDED_RECT previewShape = { preview, 4.0f, 4.0f };
        SetBrushColor(13, 17, 21, 0.88f * eased);
        g_renderTarget->FillRoundedRectangle(&previewShape, g_brush);
        // Bright white outline around the preview frame.
        SetBrushColor(245, 247, 250, 0.92f * eased);
        g_renderTarget->DrawRoundedRectangle(&previewShape, g_brush, 1.25f, NULL);

        D2D1_SIZE_F size = g_previewBitmap->GetSize();
        float sx = (preview.right-preview.left)/size.width;
        float sy = (preview.bottom-preview.top)/size.height;
        float scale = sx < sy ? sx : sy;
        float dw = size.width * scale, dh = size.height * scale;
        D2D1_RECT_F dest = { preview.left + ((preview.right-preview.left)-dw)/2.0f, preview.top + ((preview.bottom-preview.top)-dh)/2.0f,
                            preview.left + ((preview.right-preview.left)+dw)/2.0f, preview.top + ((preview.bottom-preview.top)+dh)/2.0f };
        g_renderTarget->DrawBitmap(g_previewBitmap, &dest, eased, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);

        // Thin white outline around the actual image area.
        D2D1_ROUNDED_RECT imageOutline = { dest, 3.0f, 3.0f };
        SetBrushColor(255, 255, 255, 0.78f * eased);
        g_renderTarget->DrawRoundedRectangle(&imageOutline, g_brush, 1.0f, NULL);

        if (eased > 0.35f)
        {
            const float saveScale = 1.0f - 0.035f * SmoothStep(g_savePress);
            D2D1_RECT_F animatedSaveButton = ScaleRectFromCenter(saveButton, saveScale);
            D2D1_ROUNDED_RECT saveShape = { animatedSaveButton, 4.0f, 4.0f };
            const bool hoverSave = PointInRectF(g_mouseX, g_mouseY, saveButtonBase);
            SetBrushColor(hoverSave ? 255 : 235, hoverSave ? 255 : 238, hoverSave ? 255 : 241, eased);
            g_renderTarget->FillRoundedRectangle(&saveShape, g_brush);
            SetBrushColor(25, 29, 34, eased);
            DrawDownloadIcon(animatedSaveButton.left + 20.0f, animatedSaveButton.top + 17.0f);
            DrawCenteredText(L"Save thumbnail", {animatedSaveButton.left+28.0f,animatedSaveButton.top,animatedSaveButton.right-5.0f,animatedSaveButton.bottom}, 25,29,34,eased);

            // Red Close button beside Save. It discards the preview without saving.
            D2D1_RECT_F closeBase = GetPreviewCloseRect(width);
            const float closeOffset = (1.0f - eased) * 12.0f;
            D2D1_RECT_F closeButton = { closeBase.left, closeBase.top + closeOffset, closeBase.right, closeBase.bottom + closeOffset };
            const bool hoverClose = PointInRectF(g_mouseX, g_mouseY, closeBase);
            D2D1_ROUNDED_RECT closeShape = { closeButton, 4.0f, 4.0f };
            SetBrushColor(hoverClose ? 239 : 205, hoverClose ? 67 : 52, hoverClose ? 67 : 52, eased);
            g_renderTarget->FillRoundedRectangle(&closeShape, g_brush);
            SetBrushColor(255, 125, 125, 0.55f * eased);
            g_renderTarget->DrawRoundedRectangle(&closeShape, g_brush, 1.0f, NULL);
            SetBrushColor(255, 255, 255, eased);
            DrawTrashIcon(closeButton.left + 20.0f, closeButton.top + 18.0f);
            DrawCenteredText(L"Close", {closeButton.left+31.0f,closeButton.top,closeButton.right-5.0f,closeButton.bottom}, 255,255,255,eased);
        }

        if (!g_statusText.empty())
            DrawCenteredText(g_statusText.c_str(), {SIDEBAR_WIDTH,saveButtonBase.bottom+8.0f,width,saveButtonBase.bottom+38.0f}, 150,157,164,0.9f * eased);
    }
    else if (!g_statusText.empty() && !g_clearPreviewAfterAnimation)
    {
        DrawCenteredText(g_statusText.c_str(), {SIDEBAR_WIDTH,80.0f,width,120.0f}, 150,157,164,0.9f);
    }
}

static float& ActiveLibraryScroll(bool favoritesOnly)
{
    return favoritesOnly ? g_favoritesScroll : g_libraryScroll;
}

static int CountVisibleLibraryItems(bool favoritesOnly)
{
    if (!favoritesOnly) return static_cast<int>(g_library.size());
    int count = 0;
    for (size_t i = 0; i < g_library.size(); ++i)
        if (g_library[i].favorite) ++count;
    return count;
}

static float LibraryViewportTop() { return 20.0f; }
static float LibraryViewportBottom(float height) { return height - 18.0f; }

static float LibraryMaxScroll(float height, bool favoritesOnly)
{
    const int columns = 3;
    const int count = CountVisibleLibraryItems(favoritesOnly);
    const int rows = (count + columns - 1) / columns;
    const float cardHeight = 238.0f;
    const float gap = 18.0f;
    const float contentHeight = rows > 0 ? rows * cardHeight + (rows - 1) * gap : 0.0f;
    const float viewportHeight = LibraryViewportBottom(height) - LibraryViewportTop();
    return (std::max)(0.0f, contentHeight - viewportHeight);
}

static void ClampLibraryScroll(float height, bool favoritesOnly)
{
    float& scroll = ActiveLibraryScroll(favoritesOnly);
    const float maxScroll = LibraryMaxScroll(height, favoritesOnly);
    if (scroll < 0.0f) scroll = 0.0f;
    if (scroll > maxScroll) scroll = maxScroll;
}

static D2D1_RECT_F LibraryCardRect(float width, int visibleIndex, bool favoritesOnly)
{
    const float gap = 18.0f;
    const float margin = 24.0f;
    const int columns = 3;
    const float scrollbarSpace = 12.0f;
    const float contentWidth = width - SIDEBAR_WIDTH - margin * 2.0f - scrollbarSpace;
    const float cardWidth = (contentWidth - gap * (columns - 1)) / columns;
    const float cardHeight = 238.0f;
    const int column = visibleIndex % columns;
    const int row = visibleIndex / columns;
    const float left = SIDEBAR_WIDTH + margin + column * (cardWidth + gap);
    const float top = LibraryViewportTop() + row * (cardHeight + gap) - ActiveLibraryScroll(favoritesOnly);
    return { left, top, left + cardWidth, top + cardHeight };
}

static D2D1_RECT_F LibraryFavoriteRect(const D2D1_RECT_F& card)
{
    return { card.left + 12.0f, card.bottom - 42.0f, card.left + 48.0f, card.bottom - 10.0f };
}

static D2D1_RECT_F LibraryDeleteRect(const D2D1_RECT_F& card)
{
    return { card.right - 48.0f, card.bottom - 42.0f, card.right - 12.0f, card.bottom - 10.0f };
}

static D2D1_RECT_F LibraryFolderRect(const D2D1_RECT_F& card)
{
    const float center = (card.left + card.right) * 0.5f;
    return { center - 18.0f, card.bottom - 42.0f, center + 18.0f, card.bottom - 10.0f };
}

static D2D1_RECT_F LibraryScrollbarTrackRect(float width, float height)
{
    return { width - 9.0f, LibraryViewportTop(), width - 5.0f, LibraryViewportBottom(height) };
}

static D2D1_RECT_F LibraryScrollbarThumbRect(float width, float height, bool favoritesOnly)
{
    D2D1_RECT_F track = LibraryScrollbarTrackRect(width, height);
    const float maxScroll = LibraryMaxScroll(height, favoritesOnly);
    if (maxScroll <= 0.0f) return {0,0,0,0};
    const float trackHeight = track.bottom - track.top;
    const float viewportHeight = LibraryViewportBottom(height) - LibraryViewportTop();
    const float contentHeight = viewportHeight + maxScroll;
    const float thumbHeight = (std::max)(34.0f, trackHeight * viewportHeight / contentHeight);
    const float travel = trackHeight - thumbHeight;
    const float ratio = ActiveLibraryScroll(favoritesOnly) / maxScroll;
    const float top = track.top + travel * ratio;
    return { track.left, top, track.right, top + thumbHeight };
}

static void DrawLibraryScrollbar(float width, float height, bool favoritesOnly)
{
    D2D1_RECT_F thumb = LibraryScrollbarThumbRect(width, height, favoritesOnly);
    if (thumb.right <= thumb.left) return;
    const bool hover = PointInRectF(g_mouseX, g_mouseY, thumb);
    D2D1_ROUNDED_RECT rounded = { thumb, 2.0f, 2.0f };
    SetBrushColor(255,255,255, hover || g_scrollbarDragging ? 0.98f : 0.78f);
    g_renderTarget->FillRoundedRectangle(&rounded, g_brush);
}

static void DrawLibraryPage(float width, float height, bool favoritesOnly)
{
    EnsureLibraryBitmaps();
    ClampLibraryScroll(height, favoritesOnly);

    D2D1_RECT_F clip = { SIDEBAR_WIDTH + 1.0f, LibraryViewportTop(), width - 12.0f, LibraryViewportBottom(height) };
    g_renderTarget->PushAxisAlignedClip(&clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    int visible = 0;
    for (size_t i = 0; i < g_library.size(); ++i)
    {
        if (favoritesOnly && !g_library[i].favorite) continue;
        D2D1_RECT_F card = LibraryCardRect(width, visible++, favoritesOnly);
        if (card.bottom < clip.top || card.top > clip.bottom) continue;
        float cardOpacity = 1.0f;
        if (static_cast<int>(i) == g_deletingLibraryIndex)
        {
            const float t = g_libraryDeleteAnimation;
            cardOpacity = 1.0f - t;
            const float scale = 1.0f - 0.10f * t;
            const float cx = (card.left + card.right) * 0.5f;
            const float cy = (card.top + card.bottom) * 0.5f;
            const float halfW = (card.right - card.left) * 0.5f * scale;
            const float halfH = (card.bottom - card.top) * 0.5f * scale;
            card = { cx - halfW, cy - halfH, cx + halfW, cy + halfH };
        }

        D2D1_ROUNDED_RECT shape = { card, 4.0f, 4.0f };
        const bool hoverCard = PointInRectF(g_mouseX, g_mouseY, card);
        SetBrushColor(15,19,23,0.92f * cardOpacity); g_renderTarget->FillRoundedRectangle(&shape,g_brush);
        SetBrushColor(hoverCard ? 242 : 50, hoverCard ? 246 : 57, hoverCard ? 250 : 64, (hoverCard ? 0.95f : 0.82f) * cardOpacity);
        g_renderTarget->DrawRoundedRectangle(&shape,g_brush,hoverCard ? 1.35f : 1.0f,NULL);

        D2D1_RECT_F thumb = {card.left+8.0f,card.top+8.0f,card.right-8.0f,card.top+154.0f};
        if (g_library[i].bitmap != NULL)
            g_renderTarget->DrawBitmap(g_library[i].bitmap,&thumb,cardOpacity,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,NULL);

        std::wstring visibleTitle = ShortLibraryTitle(g_library[i].title);
        DrawCenteredText(visibleTitle.c_str(), {card.left+10.0f,card.top+162.0f,card.right-10.0f,card.top+190.0f}, 226,230,234,cardOpacity);

        D2D1_RECT_F favoriteRect = favoritesOnly
            ? D2D1_RECT_F{ (card.left+card.right)*0.5f-18.0f, card.bottom-42.0f, (card.left+card.right)*0.5f+18.0f, card.bottom-10.0f }
            : LibraryFavoriteRect(card);
        const bool hoverFavorite = PointInRectF(g_mouseX, g_mouseY, favoriteRect);

        if (g_library[i].favorite) SetBrushColor(235,55,72,cardOpacity);
        else if (hoverFavorite) SetBrushColor(245,248,250,cardOpacity);
        else SetBrushColor(157,164,171,cardOpacity);
        DrawHeartIcon((favoriteRect.left+favoriteRect.right)*0.5f, (favoriteRect.top+favoriteRect.bottom)*0.5f, g_library[i].favorite);

        // Favorites page intentionally shows only the heart action.
        if (!favoritesOnly)
        {
            D2D1_RECT_F deleteRect = LibraryDeleteRect(card);
            D2D1_RECT_F folderRect = LibraryFolderRect(card);
            const bool hoverFolder = PointInRectF(g_mouseX, g_mouseY, folderRect);
            const bool hoverDelete = PointInRectF(g_mouseX, g_mouseY, deleteRect);
            SetBrushColor(hoverFolder ? 245 : 172, hoverFolder ? 248 : 178, hoverFolder ? 250 : 184, cardOpacity);
            DrawFolderIcon((folderRect.left+folderRect.right)*0.5f, (folderRect.top+folderRect.bottom)*0.5f);
            SetBrushColor(hoverDelete ? 245 : 172, hoverDelete ? 248 : 178, hoverDelete ? 250 : 184, cardOpacity);
            DrawTrashIcon((deleteRect.left+deleteRect.right)*0.5f, (deleteRect.top+deleteRect.bottom)*0.5f);
        }
    }
    g_renderTarget->PopAxisAlignedClip();

    if (visible == 0)
        DrawCenteredText(favoritesOnly ? L"No favorites yet." : L"Your library is empty.", {SIDEBAR_WIDTH,0,width,height}, 130,137,144,0.9f);
    DrawLibraryScrollbar(width, height, favoritesOnly);
}

static D2D1_RECT_F GetLibraryPreviewImageRect(float width, float height)
{
    D2D1_RECT_F empty = { 0, 0, 0, 0 };
    if (g_libraryPreviewIndex < 0 || g_libraryPreviewIndex >= static_cast<int>(g_library.size())) return empty;
    LibraryItem& item = g_library[g_libraryPreviewIndex];
    if (item.bitmap == NULL) return empty;

    const float maxW = width - SIDEBAR_WIDTH - 110.0f;
    const float maxH = height - 120.0f;
    D2D1_SIZE_F bmp = item.bitmap->GetSize();
    float scale = (std::min)(maxW / bmp.width, maxH / bmp.height);
    float drawW = bmp.width * scale;
    float drawH = bmp.height * scale;
    float cx = SIDEBAR_WIDTH + (width - SIDEBAR_WIDTH) * 0.5f;
    float cy = height * 0.5f + 8.0f;
    return { cx - drawW * 0.5f, cy - drawH * 0.5f, cx + drawW * 0.5f, cy + drawH * 0.5f };
}

static void DrawLibraryFullPreview(float width, float height)
{
    if (g_libraryPreviewIndex < 0 || g_libraryPreviewIndex >= static_cast<int>(g_library.size())) return;
    LibraryItem& item = g_library[g_libraryPreviewIndex];
    if (item.bitmap == NULL) return;

    SetBrushColor(5, 7, 9, 0.88f);
    D2D1_RECT_F dim = { SIDEBAR_WIDTH + 1.0f, 0.0f, width, height };
    g_renderTarget->FillRectangle(&dim, g_brush);

    D2D1_RECT_F image = GetLibraryPreviewImageRect(width, height);
    SetBrushColor(255,255,255,0.96f);
    D2D1_RECT_F outline = { image.left - 2.0f, image.top - 2.0f, image.right + 2.0f, image.bottom + 2.0f };
    g_renderTarget->DrawRectangle(&outline, g_brush, 1.5f, NULL);
    g_renderTarget->DrawBitmap(item.bitmap, &image, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);

    DrawCenteredText(L"Click outside the image to close", { SIDEBAR_WIDTH, 24.0f, width, 58.0f }, 188, 194, 200, 0.92f);
}

static D2D1_RECT_F SettingsPanelRect(float width, float top)
{
    const float panelWidth = 620.0f;
    const float contentWidth = width - SIDEBAR_WIDTH;
    const float left = SIDEBAR_WIDTH + (contentWidth - panelWidth) * 0.5f;
    return { left, top, left + panelWidth, top + 344.0f };
}

static D2D1_RECT_F SettingsDaysRect(float width)
{
    D2D1_RECT_F panel = SettingsPanelRect(width, 116.0f);
    return { panel.right - 196.0f, panel.top + 78.0f, panel.right - 24.0f, panel.top + 118.0f };
}

static D2D1_RECT_F SettingsFolderRect(float width)
{
    D2D1_RECT_F panel = SettingsPanelRect(width, 116.0f);
    return { panel.left + 24.0f, panel.top + 224.0f, panel.right - 24.0f, panel.top + 278.0f };
}

static D2D1_RECT_F SettingsChangeFolderRect(float width)
{
    D2D1_RECT_F row = SettingsFolderRect(width);
    return { row.right - 138.0f, row.top + 9.0f, row.right - 6.0f, row.bottom - 9.0f };
}

static D2D1_RECT_F SettingsDropdownOptionRect(float width, int index)
{
    D2D1_RECT_F days = SettingsDaysRect(width);
    const float optionHeight = 32.0f;
    return { days.left, days.bottom + 6.0f + index * optionHeight,
             days.right, days.bottom + 6.0f + (index + 1) * optionHeight };
}

static const wchar_t* DeleteDaysLabel(int days)
{
    switch (days)
    {
    case 1: return L"1 day";
    case 3: return L"3 days";
    case 7: return L"7 days";
    case 14: return L"14 days";
    case 30: return L"30 days";
    case 60: return L"60 days";
    case 90: return L"90 days";
    default: return L"Never";
    }
}

static void DrawVerticallyCenteredText(const wchar_t* text, const D2D1_RECT_F& rect,
                                       BYTE r, BYTE g, BYTE b, float alpha = 1.0f)
{
    if (g_textFormat == NULL || text == NULL) return;
    g_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    g_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    DrawTextSimple(text, rect, r, g, b, alpha);
    g_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

static void DrawSettingsPage(float width, float height)
{
    (void)height;

    // Page heading.
    D2D1_RECT_F panel = SettingsPanelRect(width, 116.0f);
    DrawTextSimple(L"Settings", { panel.left, 35.0f, panel.right, 70.0f }, 242, 245, 248, 1.0f);
    DrawTextSimple(L"Manage automatic cleanup and where thumbnails are saved.",
        { panel.left, 70.0f, panel.right, 96.0f }, 132, 140, 148, 1.0f);

    // One real settings surface instead of two floating cards.
    D2D1_ROUNDED_RECT panelShape = { panel, 8.0f, 8.0f };
    SetBrushColor(14, 18, 23, 0.97f);
    g_renderTarget->FillRoundedRectangle(&panelShape, g_brush);
    SetBrushColor(55, 63, 71, 0.95f);
    g_renderTarget->DrawRoundedRectangle(&panelShape, g_brush, 1.0f, NULL);

    // Section title.
    DrawTextSimple(L"Storage", { panel.left + 24.0f, panel.top + 18.0f, panel.right - 24.0f, panel.top + 44.0f },
        232, 236, 240, 1.0f);
    DrawTextSimple(L"Control saved thumbnails and disk usage.",
        { panel.left + 24.0f, panel.top + 43.0f, panel.right - 24.0f, panel.top + 65.0f },
        123, 131, 139, 1.0f);

    // Cleanup setting row.
    const D2D1_RECT_F cleanupRow = { panel.left + 24.0f, panel.top + 69.0f, panel.right - 24.0f, panel.top + 132.0f };
    const bool hoverCleanup = PointInRectF(g_mouseX, g_mouseY, cleanupRow);
    D2D1_ROUNDED_RECT cleanupShape = { cleanupRow, 5.0f, 5.0f };
    SetBrushColor(hoverCleanup ? 21 : 18, hoverCleanup ? 26 : 23, hoverCleanup ? 31 : 28, 1.0f);
    g_renderTarget->FillRoundedRectangle(&cleanupShape, g_brush);

    DrawVerticallyCenteredText(L"Delete older thumbnails",
        { cleanupRow.left + 16.0f, cleanupRow.top + 4.0f, cleanupRow.right - 220.0f, cleanupRow.top + 34.0f },
        225, 229, 233, 1.0f);
    DrawVerticallyCenteredText(L"Automatically remove saved files after a chosen time.",
        { cleanupRow.left + 16.0f, cleanupRow.top + 30.0f, cleanupRow.right - 220.0f, cleanupRow.bottom - 4.0f },
        126, 134, 142, 1.0f);

    D2D1_RECT_F days = SettingsDaysRect(width);
    const bool hoverDays = PointInRectF(g_mouseX, g_mouseY, days);
    D2D1_ROUNDED_RECT daysShape = { days, 4.0f, 4.0f };
    SetBrushColor(11, 15, 19, 1.0f);
    g_renderTarget->FillRoundedRectangle(&daysShape, g_brush);
    SetBrushColor(hoverDays || g_daysDropdownOpen ? 230 : 70,
                  hoverDays || g_daysDropdownOpen ? 234 : 77,
                  hoverDays || g_daysDropdownOpen ? 238 : 84, 0.98f);
    g_renderTarget->DrawRoundedRectangle(&daysShape, g_brush,
        hoverDays || g_daysDropdownOpen ? 1.25f : 1.0f, NULL);
    DrawVerticallyCenteredText(DeleteDaysLabel(g_deleteOlderDays),
        { days.left + 13.0f, days.top, days.right - 38.0f, days.bottom },
        231, 235, 239, 1.0f);

    SetBrushColor(190, 197, 204, 1.0f);
    const float arrowY = (days.top + days.bottom) * 0.5f;
    if (g_daysDropdownOpen)
    {
        DrawLineAA(days.right - 24.0f, arrowY + 3.0f, days.right - 18.0f, arrowY - 3.0f, 1.5f);
        DrawLineAA(days.right - 18.0f, arrowY - 3.0f, days.right - 12.0f, arrowY + 3.0f, 1.5f);
    }
    else
    {
        DrawLineAA(days.right - 24.0f, arrowY - 3.0f, days.right - 18.0f, arrowY + 3.0f, 1.5f);
        DrawLineAA(days.right - 18.0f, arrowY + 3.0f, days.right - 12.0f, arrowY - 3.0f, 1.5f);
    }

    // Divider.
    SetBrushColor(44, 51, 58, 0.95f);
    DrawLineAA(panel.left + 24.0f, panel.top + 151.0f, panel.right - 24.0f, panel.top + 151.0f, 1.0f);

    // Folder setting.
    DrawTextSimple(L"Save location", { panel.left + 24.0f, panel.top + 170.0f, panel.right - 24.0f, panel.top + 195.0f },
        225, 229, 233, 1.0f);
    const bool usingDefaultFolder = (_wcsicmp(g_saveFolder.c_str(), DefaultThumbnailFolderPath().c_str()) == 0);
    DrawTextSimple(usingDefaultFolder ? L"Pictures / Maotaw Thumbnails (default)" : L"Custom location / Maotaw Thumbnails",
        { panel.left + 24.0f, panel.top + 196.0f, panel.right - 24.0f, panel.top + 216.0f },
        126, 134, 142, 1.0f);

    D2D1_RECT_F folder = SettingsFolderRect(width);
    const bool hoverFolder = PointInRectF(g_mouseX, g_mouseY, folder);
    D2D1_ROUNDED_RECT folderShape = { folder, 5.0f, 5.0f };
    SetBrushColor(hoverFolder ? 21 : 18, hoverFolder ? 26 : 23, hoverFolder ? 31 : 28, 1.0f);
    g_renderTarget->FillRoundedRectangle(&folderShape, g_brush);

    SetBrushColor(185, 191, 197, 1.0f);
    DrawFolderIcon(folder.left + 23.0f, (folder.top + folder.bottom) * 0.5f);

    std::wstring shown = g_saveFolder;
    if (shown.size() > 45) shown = L"..." + shown.substr(shown.size() - 42);
    DrawVerticallyCenteredText(shown.c_str(),
        { folder.left + 44.0f, folder.top, folder.right - 154.0f, folder.bottom },
        219, 223, 227, 1.0f);

    D2D1_RECT_F change = SettingsChangeFolderRect(width);
    const bool hoverChange = PointInRectF(g_mouseX, g_mouseY, change);
    D2D1_ROUNDED_RECT changeShape = { change, 4.0f, 4.0f };
    SetBrushColor(hoverChange ? 255 : 235, hoverChange ? 255 : 238, hoverChange ? 255 : 241, 1.0f);
    g_renderTarget->FillRoundedRectangle(&changeShape, g_brush);
    DrawCenteredText(L"Change folder", change, 24, 29, 34, 1.0f);

    // Dropdown is rendered last, above the rest of the menu.
    if (g_daysDropdownOpen)
    {
        const int values[] = { 0, 1, 3, 7, 14, 30, 60, 90 };
        for (int i = 0; i < 8; ++i)
        {
            D2D1_RECT_F option = SettingsDropdownOptionRect(width, i);
            const bool hover = PointInRectF(g_mouseX, g_mouseY, option);
            const bool selected = values[i] == g_deleteOlderDays;
            D2D1_ROUNDED_RECT optionShape = { option, 3.0f, 3.0f };
            SetBrushColor(hover ? 38 : (selected ? 29 : 17),
                          hover ? 45 : (selected ? 36 : 22),
                          hover ? 53 : (selected ? 43 : 27), 1.0f);
            g_renderTarget->FillRoundedRectangle(&optionShape, g_brush);
            DrawVerticallyCenteredText(DeleteDaysLabel(values[i]),
                { option.left + 13.0f, option.top, option.right - 10.0f, option.bottom },
                232, 236, 240, 1.0f);
        }
    }
}

static void DrawEmptyPage(float width, float height)
{
    if (g_activePage == 0) DrawHomePage(width, height);
    else if (g_activePage == 1) DrawLibraryPage(width, height, false);
    else if (g_activePage == 2) DrawLibraryPage(width, height, true);
    else if (g_activePage == 3) DrawSettingsPage(width, height);
}

static void DrawWindow()
{
    if (FAILED(CreateGraphics()))
        return;

    RECT client = {};
    GetClientRect(g_window, &client);
    float width = static_cast<float>(client.right - client.left);
    float height = static_cast<float>(client.bottom - client.top);

    g_renderTarget->BeginDraw();
    D2D1_MATRIX_3X2_F identity = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
    g_renderTarget->SetTransform(&identity);
    D2D1_COLOR_F clearColor = MakeColor(10, 13, 16);
    g_renderTarget->Clear(&clearColor);

    D2D1_RECT_F full = { 0.0f, 0.0f, width, height };
    g_renderTarget->FillRectangle(&full, g_gradient);

    D2D1_RECT_F sidebar = { 1.0f, 1.0f, SIDEBAR_WIDTH, height - 1.0f };
    D2D1_COLOR_F sidebarColor = MakeColor(12, 15, 18, 0.96f);
    g_brush->SetColor(&sidebarColor);
    g_renderTarget->FillRectangle(&sidebar, g_brush);

    D2D1_RECT_F divider = { SIDEBAR_WIDTH, 1.0f, SIDEBAR_WIDTH + 1.0f, height - 1.0f };
    D2D1_COLOR_F dividerColor = MakeColor(119, 126, 132, 0.60f);
    g_brush->SetColor(&dividerColor);
    g_renderTarget->FillRectangle(&divider, g_brush);

    DrawEmptyPage(width, height);
    DrawLogo();
    DrawSidebarIcons(height);
    DrawLibraryFullPreview(width, height);

    D2D1_RECT_F border = { 0.5f, 0.5f, width - 0.5f, height - 0.5f };
    D2D1_COLOR_F borderColor = MakeColor(78, 84, 90, 0.75f);
    g_brush->SetColor(&borderColor);
    g_renderTarget->DrawRectangle(&border, g_brush, 1.0f, NULL);

    HRESULT hr = g_renderTarget->EndDraw(NULL, NULL);
    if (hr == D2DERR_RECREATE_TARGET)
        DiscardGraphics();
}

