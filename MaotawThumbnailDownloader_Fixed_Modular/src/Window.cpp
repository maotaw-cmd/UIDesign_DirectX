#include "../include/Window.h"

static LRESULT BorderlessHitTest(HWND hwnd, LPARAM lParam)
{
    POINT point;
    point.x = static_cast<short>(LOWORD(lParam));
    point.y = static_cast<short>(HIWORD(lParam));

    RECT windowRect = {};
    GetWindowRect(hwnd, &windowRect);

    const LONG grip = 6;
    BOOL left = point.x < windowRect.left + grip;
    BOOL right = point.x >= windowRect.right - grip;
    BOOL top = point.y < windowRect.top + grip;
    BOOL bottom = point.y >= windowRect.bottom - grip;

    if (top && left) return HTTOPLEFT;
    if (top && right) return HTTOPRIGHT;
    if (bottom && left) return HTBOTTOMLEFT;
    if (bottom && right) return HTBOTTOMRIGHT;
    if (left) return HTLEFT;
    if (right) return HTRIGHT;
    if (top) return HTTOP;
    if (bottom) return HTBOTTOM;

    POINT clientPoint = point;
    ScreenToClient(hwnd, &clientPoint);
    RECT clientRect = {};
    GetClientRect(hwnd, &clientRect);
    int page = HitTestNavigation(static_cast<float>(clientPoint.x),
                                 static_cast<float>(clientPoint.y),
                                 static_cast<float>(clientRect.bottom));
    if (page >= 0)
        return HTCLIENT;

    if (g_activePage == 0)
    {
        float x = static_cast<float>(clientPoint.x);
        float y = static_cast<float>(clientPoint.y);
        float width = static_cast<float>(clientRect.right);
        if (PointInRectF(x, y, GetSearchInputRect(width)) ||
            PointInRectF(x, y, GetDownloadButtonRect(width)) ||
            (g_previewBitmap != NULL && (PointInRectF(x, y, GetSaveButtonRect(width)) ||
                                          PointInRectF(x, y, GetPreviewCloseRect(width)))))
            return HTCLIENT;
    }

    if (g_activePage == 1 || g_activePage == 2 || g_activePage == 3) return HTCLIENT;
    return HTCAPTION;
}

static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCPAINT:
        // The complete window is painted by Direct2D; never let Windows draw
        // a native non-client border over it.
        return 0;

    case WM_NCACTIVATE:
        // Prevent the active/inactive native frame from appearing.
        return TRUE;

    case WM_ERASEBKGND:
        // Direct2D clears the full client area during WM_PAINT.
        return 1;

    case WM_NCCALCSIZE:
        if (wParam != 0)
            return 0;
        break;

    case WM_NCHITTEST:
        return BorderlessHitTest(hwnd, lParam);

    case WM_SETCURSOR:
    {
        POINT pt = {};
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        RECT rc = {};
        GetClientRect(hwnd, &rc);
        float x = static_cast<float>(pt.x);
        float y = static_cast<float>(pt.y);
        float width = static_cast<float>(rc.right);
        if (g_libraryPreviewIndex >= 0)
        {
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
            return TRUE;
        }

        int page = HitTestNavigation(x, y, static_cast<float>(rc.bottom));
        if (page >= 0)
        {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        if (g_activePage == 0 && PointInRectF(x, y, GetSearchInputRect(width)))
        {
            SetCursor(LoadCursorW(NULL, IDC_IBEAM));
            return TRUE;
        }
        if (g_activePage == 0 && (PointInRectF(x, y, GetDownloadButtonRect(width)) ||
            (g_previewBitmap != NULL && (PointInRectF(x, y, GetSaveButtonRect(width)) ||
                                         PointInRectF(x, y, GetPreviewCloseRect(width))))))
        {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        if (g_libraryPreviewIndex < 0 && (g_activePage == 1 || g_activePage == 2))
        {
            const bool favoritesOnly = g_activePage == 2;
            D2D1_RECT_F scrollThumb = LibraryScrollbarThumbRect(width, static_cast<float>(rc.bottom), favoritesOnly);
            if (PointInRectF(x, y, scrollThumb))
            {
                SetCursor(LoadCursorW(NULL, IDC_HAND));
                return TRUE;
            }
            int visible = 0;
            for (size_t i = 0; i < g_library.size(); ++i)
            {
                if (g_activePage == 2 && !g_library[i].favorite) continue;
                D2D1_RECT_F card = LibraryCardRect(width, visible++, g_activePage == 2);
                if (PointInRectF(x, y, card))
                {
                    SetCursor(LoadCursorW(NULL, IDC_HAND));
                    return TRUE;
                }
            }
        }
        bool hoverSettingsControl = false;
        if (g_activePage == 3)
        {
            hoverSettingsControl = PointInRectF(x, y, SettingsDaysRect(width)) ||
                                   PointInRectF(x, y, SettingsFolderRect(width)) ||
                                   PointInRectF(x, y, SettingsChangeFolderRect(width));
            if (g_daysDropdownOpen)
            {
                for (int i = 0; i < 8 && !hoverSettingsControl; ++i)
                    hoverSettingsControl = PointInRectF(x, y, SettingsDropdownOptionRect(width, i));
            }
        }
        if (hoverSettingsControl)
        {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        break;
    }

    case WM_MOUSEMOVE:
        g_mouseX = static_cast<float>(static_cast<short>(LOWORD(lParam)));
        g_mouseY = static_cast<float>(static_cast<short>(HIWORD(lParam)));
        if (g_scrollbarDragging && (g_activePage == 1 || g_activePage == 2))
        {
            RECT rc = {}; GetClientRect(hwnd, &rc);
            const bool favoritesOnly = g_activePage == 2;
            D2D1_RECT_F track = LibraryScrollbarTrackRect(static_cast<float>(rc.right), static_cast<float>(rc.bottom));
            D2D1_RECT_F thumb = LibraryScrollbarThumbRect(static_cast<float>(rc.right), static_cast<float>(rc.bottom), favoritesOnly);
            const float thumbHeight = thumb.bottom - thumb.top;
            const float travel = (track.bottom - track.top) - thumbHeight;
            if (travel > 0.0f)
            {
                float thumbTop = g_mouseY - g_scrollbarDragOffset;
                thumbTop = (std::max)(track.top, (std::min)(thumbTop, track.bottom - thumbHeight));
                ActiveLibraryScroll(favoritesOnly) = ((thumbTop - track.top) / travel) * LibraryMaxScroll(static_cast<float>(rc.bottom), favoritesOnly);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        if (g_inputMouseSelecting && g_inputFocused && g_activePage == 0)
        {
            RECT rc = {}; GetClientRect(hwnd, &rc);
            D2D1_RECT_F input = GetSearchInputRect(static_cast<float>(rc.right));
            const float textLeft = input.left + 32.0f;
            const float textRight = input.right - 10.0f;

            // While dragging beyond either side, scroll the one-line input so
            // the user can keep selecting text that is currently hidden.
            if (g_mouseX < textLeft)
            {
                if (g_inputViewStart > 0) --g_inputViewStart;
                g_inputCaret = g_inputViewStart;
            }
            else if (g_mouseX > textRight)
            {
                const size_t visibleEnd = (std::min)(
                    g_inputViewStart + INPUT_VISIBLE_CHARACTERS,
                    g_searchText.size());
                if (visibleEnd < g_searchText.size()) ++g_inputViewStart;
                g_inputCaret = (std::min)(
                    g_inputViewStart + INPUT_VISIBLE_CHARACTERS,
                    g_searchText.size());
            }
            else
            {
                g_inputCaret = InputIndexFromMouseX(g_mouseX, input);
            }

            NormalizeInputSelection();
            g_caretVisible = true;
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_MOUSELEAVE:
        g_mouseX = -1000.0f;
        g_mouseY = -1000.0f;
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_MOUSEWHEEL:
        if (g_activePage == 1 || g_activePage == 2)
        {
            RECT rc = {}; GetClientRect(hwnd, &rc);
            const bool favoritesOnly = g_activePage == 2;
            const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            ActiveLibraryScroll(favoritesOnly) -= (static_cast<float>(delta) / WHEEL_DELTA) * 72.0f;
            ClampLibraryScroll(static_cast<float>(rc.bottom), favoritesOnly);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;

    case WM_LBUTTONDOWN:
    {
        float x = static_cast<float>(static_cast<short>(LOWORD(lParam)));
        float y = static_cast<float>(static_cast<short>(HIWORD(lParam)));
        RECT rc = {};
        GetClientRect(hwnd, &rc);
        float width = static_cast<float>(rc.right);
        int page = HitTestNavigation(x, y, static_cast<float>(rc.bottom));
        if (page >= 0)
        {
            g_activePage = page;
            g_scrollbarDragging = false;
            g_inputFocused = false;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        if (g_libraryPreviewIndex >= 0)
        {
            D2D1_RECT_F image = GetLibraryPreviewImageRect(width, static_cast<float>(rc.bottom));
            if (!PointInRectF(x, y, image))
            {
                g_libraryPreviewIndex = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        if (g_activePage == 0)
        {
            D2D1_RECT_F input = GetSearchInputRect(width);
            D2D1_RECT_F download = GetDownloadButtonRect(width);

            if (PointInRectF(x, y, input))
            {
                g_inputFocused = true;
                g_caretVisible = true;
                SetFocus(hwnd);
                g_inputCaret = InputIndexFromMouseX(x, input);
                if ((GetKeyState(VK_SHIFT) & 0x8000) == 0) g_inputAnchor = g_inputCaret;
                g_inputMouseSelecting = true;
                SetCapture(hwnd);
                NormalizeInputSelection();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (PointInRectF(x, y, download))
            {
                g_inputFocused = false;
                g_downloadPress = 1.0f;
                g_previewTarget = 0.0f;
                g_previewAnimation = 0.0f;
                g_clearPreviewAfterAnimation = false;
                SafeRelease(&g_previewBitmap);
                if (!g_previewPath.empty()) DeleteFileW(g_previewPath.c_str());
                g_previewPath.clear();
                g_statusText.clear();
                NormalizeInputSelection();
                if (FetchPreview())
                {
                    g_searchText.clear();
                    g_inputCaret = g_inputAnchor = g_inputViewStart = 0;
                    g_inputFocused = false;
                    g_previewAnimation = 0.0f;
                    g_previewTarget = 1.0f;
                }
                else
                {
                    // Keep the failed link visible and selected so the next paste replaces it.
                    g_inputFocused = true;
                    g_inputAnchor = 0;
                    g_inputCaret = g_searchText.size();
                    NormalizeInputSelection();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_previewBitmap != NULL && g_previewAnimation > 0.80f && PointInRectF(x, y, GetPreviewCloseRect(width)))
            {
                // Discard the preview smoothly without saving it.
                g_inputFocused = false;
                g_statusText.clear();
                g_previewTarget = 0.0f;
                g_clearPreviewAfterAnimation = true;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_previewBitmap != NULL && g_previewAnimation > 0.80f && PointInRectF(x, y, GetSaveButtonRect(width)))
            {
                g_inputFocused = false;
                g_savePress = 1.0f;
                if (SavePreviewToLibrary())
                {
                    g_previewTarget = 0.0f;
                    g_clearPreviewAfterAnimation = true;

                    // Make the input immediately ready for the next link while
                    // the saved preview animates out.
                    g_searchText.clear();
                    g_inputCaret = 0;
                    g_inputAnchor = 0;
                    g_inputViewStart = 0;
                    g_inputFocused = true;
                    g_caretVisible = true;
                    SetFocus(hwnd);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }

        if (g_activePage == 3)
        {
            D2D1_RECT_F days = SettingsDaysRect(width);

            // Check dropdown options first, because they visually overlap the panel below.
            if (g_daysDropdownOpen)
            {
                const int values[] = { 0, 1, 3, 7, 14, 30, 60, 90 };
                for (int i = 0; i < 8; ++i)
                {
                    if (PointInRectF(x, y, SettingsDropdownOptionRect(width, i)))
                    {
                        g_deleteOlderDays = values[i];
                        g_daysDropdownOpen = false;
                        SaveSettingsState();
                        DeleteOldThumbnailsNow();
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            }

            if (PointInRectF(x, y, days))
            {
                g_daysDropdownOpen = !g_daysDropdownOpen;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            if (PointInRectF(x, y, SettingsFolderRect(width)))
            {
                g_daysDropdownOpen = false;
                ChooseSaveFolder(hwnd);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            g_daysDropdownOpen = false;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        if (g_activePage == 1 || g_activePage == 2)
        {
            const bool favoritesOnly = g_activePage == 2;
            D2D1_RECT_F scrollThumb = LibraryScrollbarThumbRect(width, static_cast<float>(rc.bottom), favoritesOnly);
            if (PointInRectF(x, y, scrollThumb))
            {
                g_scrollbarDragging = true;
                g_scrollbarDragOffset = y - scrollThumb.top;
                SetCapture(hwnd);
                return 0;
            }
            int visible = 0;
            for (size_t i = 0; i < g_library.size(); ++i)
            {
                if (g_activePage == 2 && !g_library[i].favorite) continue;
                D2D1_RECT_F card = LibraryCardRect(width, visible++, g_activePage == 2);
                D2D1_RECT_F favoriteRect = favoritesOnly
                    ? D2D1_RECT_F{ (card.left+card.right)*0.5f-18.0f, card.bottom-42.0f, (card.left+card.right)*0.5f+18.0f, card.bottom-10.0f }
                    : LibraryFavoriteRect(card);
                if (PointInRectF(x, y, favoriteRect))
                {
                    g_library[i].favorite = !g_library[i].favorite;
                    SaveLibraryState();
                    ClampLibraryScroll(static_cast<float>(rc.bottom), favoritesOnly);
                    InvalidateRect(hwnd,NULL,FALSE); return 0;
                }
                if (!favoritesOnly && PointInRectF(x, y, LibraryFolderRect(card)))
                {
                    OpenContainingFolder(g_library[i].path);
                    return 0;
                }
                if (!favoritesOnly && PointInRectF(x, y, LibraryDeleteRect(card)))
                {
                    if (g_deletingLibraryIndex < 0)
                    {
                        g_deletingLibraryIndex = static_cast<int>(i);
                        g_libraryDeleteAnimation = 0.0f;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                }
                if (PointInRectF(x, y, card))
                {
                    g_libraryPreviewIndex = static_cast<int>(i);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
        }

        g_inputFocused = false;
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }


    case WM_LBUTTONUP:
        if (g_scrollbarDragging)
        {
            g_scrollbarDragging = false;
            ReleaseCapture();
            return 0;
        }
        if (g_inputMouseSelecting)
        {
            g_inputMouseSelecting = false;
            ReleaseCapture();
            return 0;
        }
        break;

    case WM_CHAR:
        if (g_inputFocused && g_activePage == 0)
        {
            if (wParam == 8)
            {
                if (HasInputSelection()) DeleteInputSelection();
                else if (g_inputCaret > 0)
                {
                    g_searchText.erase(g_inputCaret - 1, 1);
                    --g_inputCaret;
                    g_inputAnchor = g_inputCaret;
                }
            }
            else if (wParam >= 32 && wParam != 127 && g_searchText.size() < 180)
            {
                DeleteInputSelection();
                g_searchText.insert(g_inputCaret, 1, static_cast<wchar_t>(wParam));
                ++g_inputCaret;
                g_inputAnchor = g_inputCaret;
            }
            NormalizeInputSelection();
            g_caretVisible = true;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;

    case WM_TIMER:
        if (wParam == 1 && g_inputFocused && g_activePage == 0)
        {
            g_caretVisible = !g_caretVisible;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        if (wParam == 2)
        {
            bool changed = false;

            const float previewSpeed = 0.11f;
            if (g_previewAnimation < g_previewTarget)
            {
                g_previewAnimation += previewSpeed;
                if (g_previewAnimation > g_previewTarget) g_previewAnimation = g_previewTarget;
                changed = true;
            }
            else if (g_previewAnimation > g_previewTarget)
            {
                g_previewAnimation -= previewSpeed;
                if (g_previewAnimation < g_previewTarget) g_previewAnimation = g_previewTarget;
                changed = true;
            }

            if (g_downloadPress > 0.0f)
            {
                g_downloadPress -= 0.14f;
                if (g_downloadPress < 0.0f) g_downloadPress = 0.0f;
                changed = true;
            }
            if (g_savePress > 0.0f)
            {
                g_savePress -= 0.14f;
                if (g_savePress < 0.0f) g_savePress = 0.0f;
                changed = true;
            }

            if (g_clearPreviewAfterAnimation && g_previewAnimation <= 0.001f)
            {
                SafeRelease(&g_previewBitmap);
                if (!g_previewPath.empty()) DeleteFileW(g_previewPath.c_str());
                g_previewPath.clear();
                g_statusText.clear();
                g_clearPreviewAfterAnimation = false;
                changed = true;
            }

            if (g_deletingLibraryIndex >= 0)
            {
                g_libraryDeleteAnimation += 0.075f;
                if (g_libraryDeleteAnimation >= 1.0f)
                {
                    if (g_deletingLibraryIndex < static_cast<int>(g_library.size()))
                    {
                        // Remember the deleted file path and remove every matching
                        // library entry. This also removes duplicate/stale favorite
                        // entries that may have been restored from an older state file.
                        const std::wstring deletedPath = g_library[g_deletingLibraryIndex].path;
                        DeleteFileW(deletedPath.c_str());

                        for (size_t removeIndex = g_library.size(); removeIndex-- > 0; )
                        {
                            if (_wcsicmp(g_library[removeIndex].path.c_str(), deletedPath.c_str()) == 0)
                            {
                                SafeRelease(&g_library[removeIndex].bitmap);
                                g_library.erase(g_library.begin() + removeIndex);
                            }
                        }

                        // A full preview must never keep pointing at a removed item.
                        g_libraryPreviewIndex = -1;
                        SaveLibraryState();
                    }
                    g_deletingLibraryIndex = -1;
                    g_libraryDeleteAnimation = 0.0f;
                }
                changed = true;
            }

            if (changed) InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        break;

    case WM_SIZE:
        DiscardGraphics();
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT paint = {};
        BeginPaint(hwnd, &paint);
        DrawWindow();
        EndPaint(hwnd, &paint);
        return 0;
    }

    case WM_KEYDOWN:
        if (g_inputFocused && g_activePage == 0)
        {
            const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            const bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (ctrl && wParam == 'A')
            {
                g_inputAnchor = 0;
                g_inputCaret = g_searchText.size();
                NormalizeInputSelection();
                InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
            if (ctrl && wParam == 'V')
            {
                PasteClipboardText(); g_caretVisible = true;
                InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
            if (ctrl && wParam == 'C') { CopyInputToClipboard(false); return 0; }
            if (ctrl && wParam == 'X')
            {
                CopyInputToClipboard(true); g_caretVisible = true;
                InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
            if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_HOME || wParam == VK_END)
            {
                size_t next = g_inputCaret;
                if (wParam == VK_LEFT && next > 0) --next;
                if (wParam == VK_RIGHT && next < g_searchText.size()) ++next;
                if (wParam == VK_HOME) next = 0;
                if (wParam == VK_END) next = g_searchText.size();
                g_inputCaret = next;
                if (!shift) g_inputAnchor = g_inputCaret;
                NormalizeInputSelection();
                g_caretVisible = true;
                InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
            if (wParam == VK_DELETE)
            {
                if (HasInputSelection()) DeleteInputSelection();
                else if (g_inputCaret < g_searchText.size()) g_searchText.erase(g_inputCaret, 1);
                NormalizeInputSelection();
                InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
        }
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        SaveSettingsState();
        SaveLibraryState();
        DiscardGraphics();
        SafeRelease(&g_textFormat);
        SafeRelease(&g_writeFactory);
        SafeRelease(&g_wicFactory);
        SafeRelease(&g_factory);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

int RunApplication(HINSTANCE instance, int showCommand)
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_wicFactory));

    D2D1_FACTORY_OPTIONS options = {};

    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(ID2D1Factory),
        &options,
        reinterpret_cast<void**>(&g_factory));

    if (FAILED(hr))
        return 1;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&g_writeFactory));
    if (FAILED(hr))
        return 1;

    hr = g_writeFactory->CreateTextFormat(
        L"Segoe UI",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        12.0f,
        L"en-us",
        &g_textFormat);
    if (FAILED(hr))
        return 1;

    g_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    g_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    const wchar_t CLASS_NAME[] = L"EmptyDirect2DWindowSidebarNav";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.lpszClassName = CLASS_NAME;

    if (RegisterClassExW(&windowClass) == 0)
        return 1;

    int x = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2;

    g_window = CreateWindowExW(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        L"",
        WS_POPUP,
        x,
        y,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        instance,
        NULL);

    if (g_window == NULL)
        return 1;

    LoadSettingsState();
    LoadLibraryState();
    RecoverLibraryFromSaveFolder();
    SaveLibraryState();
    DeleteOldThumbnailsNow();

    // Keep the window fully client-drawn. Extending the DWM frame or using
    // WS_THICKFRAME can make Windows draw a wide white border after focus,
    // resizing, snapping, or a display/DPI change.
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
    const COLORREF darkFrameColor = RGB(8, 10, 16);
    DwmSetWindowAttribute(g_window, DWMWA_BORDER_COLOR,
                          &darkFrameColor, sizeof(darkFrameColor));
    DwmSetWindowAttribute(g_window, DWMWA_CAPTION_COLOR,
                          &darkFrameColor, sizeof(darkFrameColor));

    ShowWindow(g_window, showCommand);
    UpdateWindow(g_window);
    SetTimer(g_window, 1, 530, NULL);
    SetTimer(g_window, 2, 16, NULL);

    MSG message = {};
    while (GetMessageW(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    CoUninitialize();
    return static_cast<int>(message.wParam);
}
