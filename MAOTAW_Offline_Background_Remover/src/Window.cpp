#include "App.h"

LRESULT Hit(HWND h, LPARAM lp)
{
    POINT p = { static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp)) };
    ScreenToClient(h, &p);

    RECT r = {};
    GetClientRect(h, &r);

    int e = 6;
    bool l = p.x < e;
    bool rr = p.x > r.right - e;
    bool t = p.y < e;
    bool b = p.y > r.bottom - e;

    if (t && l) return HTTOPLEFT;
    if (t && rr) return HTTOPRIGHT;
    if (b && l) return HTBOTTOMLEFT;
    if (b && rr) return HTBOTTOMRIGHT;
    if (l) return HTLEFT;
    if (rr) return HTRIGHT;
    if (t) return HTTOP;
    if (b) return HTBOTTOM;

    if (g_showSettings && Inside(static_cast<float>(p.x), static_cast<float>(p.y), SettingsPopupRect(static_cast<float>(r.right))))
    {
        return HTCLIENT;
    }

    if (p.y < 70 && p.x < r.right - 92)
    {
        return HTCAPTION;
    }

    return HTCLIENT;
}

LRESULT CALLBACK Proc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m)
    {
    case WM_NCCALCSIZE:
        if (w)
        {
            return 0;
        }
        break;

    case WM_NCPAINT:
        return 0;

    case WM_NCACTIVATE:
        return TRUE;

    case WM_ERASEBKGND:
        return 1;

    case WM_NCHITTEST:
        return Hit(h, l);

    case WM_APP + 1:
        InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_APP + 2:
        if (w)
        {
            LoadEditableOutput(g_outputPath);
            g_resultAnim = 0;
        }
        InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_MOUSEMOVE:
    {
        g_mouseX = static_cast<float>(static_cast<short>(LOWORD(l)));
        g_mouseY = static_cast<float>(static_cast<short>(HIWORD(l)));

        RECT rc = {};
        GetClientRect(h, &rc);
        float ww = static_cast<float>(rc.right);

        if (g_dragSlider)
        {
            UpdateFeatherFromX(g_mouseX, ww);
            InvalidateRect(h, nullptr, FALSE);
            return 0;
        }

        InvalidateRect(h, nullptr, FALSE);
        return 0;
    }

    case WM_LBUTTONUP:
        g_dragSlider = false;
        return 0;

    case WM_SETCURSOR:
    {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(h, &p);

        RECT rc = {};
        GetClientRect(h, &rc);
        float ww = static_cast<float>(rc.right);

        bool hand = false;

        if (Inside(static_cast<float>(p.x), static_cast<float>(p.y), SettingsButtonRect(ww)))
        {
            hand = true;
        }

        if (g_showSettings && Inside(static_cast<float>(p.x), static_cast<float>(p.y), SettingsPopupRect(ww)))
        {
            hand = true;
        }

        if (g_ready && !g_inputBitmap)
        {
            hand = hand ||
                   Inside(static_cast<float>(p.x), static_cast<float>(p.y), ChooseCardRect(ww)) ||
                   Inside(static_cast<float>(p.x), static_cast<float>(p.y), ChooseSelectButtonRect(ww));
        }

        if (g_inputBitmap)
        {
            hand = hand ||
                   Inside(static_cast<float>(p.x), static_cast<float>(p.y), MainButton(ww)) ||
                   Inside(static_cast<float>(p.x), static_cast<float>(p.y), ChangeButton()) ||
                   (g_outputBitmap && Inside(static_cast<float>(p.x), static_cast<float>(p.y), SaveButton(ww)));
        }

        if (hand)
        {
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return TRUE;
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        float x = static_cast<float>(static_cast<short>(LOWORD(l)));
        float y = static_cast<float>(static_cast<short>(HIWORD(l)));

        RECT rc = {};
        GetClientRect(h, &rc);
        float ww = static_cast<float>(rc.right);

        if (Inside(x, y, SettingsButtonRect(ww)))
        {
            g_showSettings = !g_showSettings;
            InvalidateRect(h, nullptr, FALSE);
            return 0;
        }

        if (g_showSettings && Inside(x, y, FeatherTrackRect(ww)))
        {
            g_dragSlider = true;
            UpdateFeatherFromX(x, ww);
            InvalidateRect(h, nullptr, FALSE);
            return 0;
        }

        if (g_showSettings &&
            !Inside(x, y, SettingsPopupRect(ww)) &&
            !Inside(x, y, SettingsButtonRect(ww)))
        {
            g_showSettings = false;
            InvalidateRect(h, nullptr, FALSE);
        }

        if (g_eraserMode && g_outputBitmap && RefineSpotAtPoint(x, y, ww))
        {
            g_status = L"";
            InvalidateRect(h, nullptr, FALSE);
            return 0;
        }

        if (g_ready && !g_inputBitmap)
        {
            D2D1_RECT_F d = ChooseCardRect(ww);
            if (Inside(x, y, d) || Inside(x, y, ChooseSelectButtonRect(ww)))
            {
                ChooseImage();
                return 0;
            }
        }
        else if (g_inputBitmap)
        {
            if (Inside(x, y, MainButton(ww)))
            {
                Process();
                return 0;
            }

            if (Inside(x, y, ChangeButton()))
            {
                ChooseImage();
                return 0;
            }

            if (g_outputBitmap && Inside(x, y, SaveButton(ww)))
            {
                Save();
                return 0;
            }
        }
        break;
    }

    case WM_TIMER:
        if (w == 1)
        {
            g_borderPhase += 0.0065f;
            if (g_borderPhase >= 1.0f)
            {
                g_borderPhase -= 1.0f;
            }

            float gp = g_progress.load();
            float gt = g_progressTarget.load();
            if (gp < gt)
            {
                gp = (std::min)(gt, gp + 0.0025f);
                g_progress = gp;
            }

            if (g_outputBitmap && g_resultAnim < 1.0f)
            {
                g_resultAnim = (std::min)(1.0f, g_resultAnim + 0.08f);
            }

            InvalidateRect(h, nullptr, FALSE);
        }
        return 0;

    case WM_SIZE:
        Release(&g_gradient);
        Release(&g_stops);
        Release(&g_brush);
        Release(&g_rt);
        InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT p;
        BeginPaint(h, &p);
        Paint();
        EndPaint(h, &p);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(h, m, w, l);
}
