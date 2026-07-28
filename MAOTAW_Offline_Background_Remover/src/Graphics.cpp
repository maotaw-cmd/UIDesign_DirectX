#include "App.h"

HRESULT Graphics()
{
    if (g_rt)
    {
        return S_OK;
    }

    RECT rc = {};
    GetClientRect(g_hwnd, &rc);

    UINT w = static_cast<UINT>(std::max(1L, rc.right));
    UINT h = static_cast<UINT>(std::max(1L, rc.bottom));

    D2D1_RENDER_TARGET_PROPERTIES rp = {};
    rp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    rp.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
    rp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    rp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

    D2D1_HWND_RENDER_TARGET_PROPERTIES hp = {};
    hp.hwnd = g_hwnd;
    hp.pixelSize = { w, h };
    hp.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;

    HRESULT hr = g_d2d->CreateHwndRenderTarget(&rp, &hp, &g_rt);
    if (FAILED(hr))
    {
        return hr;
    }

    D2D1_COLOR_F white = C(255, 255, 255);
    hr = g_rt->CreateSolidColorBrush(&white, nullptr, &g_brush);
    if (FAILED(hr))
    {
        return hr;
    }

    D2D1_GRADIENT_STOP s[3] = {
        { 0.0f, C(21, 25, 30) },
        { 0.48f, C(16, 20, 24) },
        { 1.0f, C(10, 13, 16) }
    };

    hr = g_rt->CreateGradientStopCollection(
        s,
        3,
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        &g_stops);
    if (FAILED(hr))
    {
        return hr;
    }

    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gp = { { 0, 0 }, { static_cast<float>(w), static_cast<float>(h) } };
    return g_rt->CreateLinearGradientBrush(&gp, nullptr, g_stops, &g_gradient);
}

HRESULT LoadBitmap(const std::wstring& path, ID2D1Bitmap** out)
{
    if (!g_wic || !g_rt || !out)
    {
        return E_FAIL;
    }

    *out = nullptr;

    IWICBitmapDecoder* d = nullptr;
    IWICBitmapFrameDecode* f = nullptr;
    IWICFormatConverter* c = nullptr;

    HRESULT hr = g_wic->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &d);

    if (SUCCEEDED(hr))
    {
        hr = d->GetFrame(0, &f);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_wic->CreateFormatConverter(&c);
    }

    if (SUCCEEDED(hr))
    {
        hr = c->Initialize(
            f,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0,
            WICBitmapPaletteTypeMedianCut);
    }

    if (SUCCEEDED(hr))
    {
        hr = g_rt->CreateBitmapFromWicBitmap(c, nullptr, out);
    }

    Release(&c);
    Release(&f);
    Release(&d);
    return hr;
}

void Text(
    const wchar_t* t,
    const D2D1_RECT_F& r,
    BYTE red,
    BYTE green,
    BYTE blue,
    float a,
    bool center)
{
    g_font->SetTextAlignment(center ? DWRITE_TEXT_ALIGNMENT_CENTER : DWRITE_TEXT_ALIGNMENT_LEADING);
    g_font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    Brush(red, green, blue, a);
    g_rt->DrawTextW(
        t,
        static_cast<UINT32>(wcslen(t)),
        g_font,
        &r,
        g_brush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL);
}

void Logo()
{
    ID2D1PathGeometry* g = nullptr;
    ID2D1GeometrySink* s = nullptr;

    if (FAILED(g_d2d->CreatePathGeometry(&g)))
    {
        return;
    }

    if (FAILED(g->Open(&s)))
    {
        g->Release();
        return;
    }

    s->BeginFigure({ 39, 21 }, D2D1_FIGURE_BEGIN_HOLLOW);
    s->AddLine({ 26, 45 });
    s->AddLine({ 52, 45 });
    s->AddLine({ 39, 21 });
    s->EndFigure(D2D1_FIGURE_END_OPEN);
    s->Close();
    s->Release();

    Brush(242, 244, 246);
    g_rt->DrawGeometry(g, g_brush, 3, nullptr);
    g->Release();
}

void ImageIcon(float cx, float cy)
{
    D2D1_RECT_F b = { cx - 10, cy - 8, cx + 10, cy + 8 };
    g_rt->DrawRectangle(&b, g_brush, 1.5f);

    D2D1_ELLIPSE sun = { { cx + 4, cy - 3 }, 2, 2 };
    g_rt->DrawEllipse(&sun, g_brush, 1.4f);

    g_rt->DrawLine({ cx - 8, cy + 5 }, { cx - 2, cy }, g_brush, 1.4f);
    g_rt->DrawLine({ cx - 2, cy }, { cx + 2, cy + 4 }, g_brush, 1.4f);
    g_rt->DrawLine({ cx + 2, cy + 4 }, { cx + 7, cy - 1 }, g_brush, 1.4f);
}

void EraserIcon(float cx, float cy)
{
    D2D1_POINT_2F p1 = { cx - 6, cy + 4 };
    D2D1_POINT_2F p2 = { cx - 1, cy - 6 };
    D2D1_POINT_2F p3 = { cx + 6, cy + 1 };
    D2D1_POINT_2F p4 = { cx + 1, cy + 10 };

    g_rt->DrawLine(p1, p2, g_brush, 1.5f);
    g_rt->DrawLine(p2, p3, g_brush, 1.5f);
    g_rt->DrawLine(p3, p4, g_brush, 1.5f);
    g_rt->DrawLine(p4, p1, g_brush, 1.5f);
    g_rt->DrawLine({ cx - 2, cy - 4 }, { cx + 4, cy + 2 }, g_brush, 1.2f);
    g_rt->DrawLine({ cx - 4, cy + 7 }, { cx + 4, cy + 7 }, g_brush, 1.2f);
}

void SettingsIcon(float cx, float cy)
{
    D2D1_ELLIPSE outer = { { cx, cy }, 7.2f, 7.2f };
    D2D1_ELLIPSE inner = { { cx, cy }, 2.5f, 2.5f };

    g_rt->DrawEllipse(&outer, g_brush, 1.45f);
    g_rt->DrawEllipse(&inner, g_brush, 1.45f);

    const float k = 0.70710678f;
    const float d[8][2] = {
        { 0, -1 }, { k, -k }, { 1, 0 }, { k, k },
        { 0, 1 }, { -k, k }, { -1, 0 }, { -k, -k }
    };

    for (int i = 0; i < 8; ++i)
    {
        g_rt->DrawLine(
            { cx + d[i][0] * 8.1f, cy + d[i][1] * 8.1f },
            { cx + d[i][0] * 10.5f, cy + d[i][1] * 10.5f },
            g_brush,
            1.6f);
    }
}

void DrawPerimeterRange(float w, float h, float start, float length, float stroke, float alpha)
{
    const float m = 1.5f;
    const float top = w - 2.0f * m;
    const float right = h - 2.0f * m;
    const float bottom = top;
    const float left = right;
    const float per = top + right + bottom + left;

    while (start < 0.0f)
    {
        start += per;
    }
    while (start >= per)
    {
        start -= per;
    }

    float remain = length;
    float pos = start;
    Brush(250, 252, 255, alpha);

    while (remain > 0.01f)
    {
        float edgePos = pos;
        int edge = 0;
        float edgeLen = top;

        if (edgePos >= top)
        {
            edgePos -= top;
            edge = 1;
            edgeLen = right;
        }
        if (edge == 1 && edgePos >= right)
        {
            edgePos -= right;
            edge = 2;
            edgeLen = bottom;
        }
        if (edge == 2 && edgePos >= bottom)
        {
            edgePos -= bottom;
            edge = 3;
            edgeLen = left;
        }

        const float take = (std::min)(remain, edgeLen - edgePos);
        D2D1_POINT_2F a = {};
        D2D1_POINT_2F b = {};

        if (edge == 0)
        {
            a = { m + edgePos, m };
            b = { m + edgePos + take, m };
        }
        else if (edge == 1)
        {
            a = { w - m, m + edgePos };
            b = { w - m, m + edgePos + take };
        }
        else if (edge == 2)
        {
            a = { w - m - edgePos, h - m };
            b = { w - m - edgePos - take, h - m };
        }
        else
        {
            a = { m, h - m - edgePos };
            b = { m, h - m - edgePos - take };
        }

        g_rt->DrawLine(a, b, g_brush, stroke);
        remain -= take;
        pos += take;

        if (pos >= per)
        {
            pos -= per;
        }
    }
}

void DrawAnimatedWindowBorder(float w, float h)
{
    Brush(255, 255, 255, 0.18f);
    D2D1_RECT_F r = { 1.5f, 1.5f, w - 1.5f, h - 1.5f };
    g_rt->DrawRectangle(&r, g_brush, 1.0f);

    const float per = 2.0f * ((w - 3.0f) + (h - 3.0f));
    DrawPerimeterRange(w, h, g_borderPhase * per, per * 0.50f, 1.8f, 0.95f);
}

void SaveIcon(float cx, float cy)
{
    g_rt->DrawLine({ cx, cy - 7 }, { cx, cy + 3 }, g_brush, 1.5f);
    g_rt->DrawLine({ cx - 4, cy }, { cx, cy + 4 }, g_brush, 1.5f);
    g_rt->DrawLine({ cx + 4, cy }, { cx, cy + 4 }, g_brush, 1.5f);
    g_rt->DrawLine({ cx - 6, cy + 7 }, { cx + 6, cy + 7 }, g_brush, 1.5f);
}

D2D1_RECT_F CalcFitRect(ID2D1Bitmap* b, const D2D1_RECT_F& a)
{
    if (!b)
    {
        return a;
    }

    D2D1_SIZE_F s = b->GetSize();
    float k = (std::min)((a.right - a.left) / s.width, (a.bottom - a.top) / s.height);
    float w = s.width * k;
    float h = s.height * k;

    return {
        a.left + ((a.right - a.left) - w) / 2,
        a.top + ((a.bottom - a.top) - h) / 2,
        a.left + ((a.right - a.left) + w) / 2,
        a.top + ((a.bottom - a.top) + h) / 2
    };
}

D2D1_RECT_F DrawBitmapFit(ID2D1Bitmap* b, const D2D1_RECT_F& a, float opacity)
{
    D2D1_RECT_F d = CalcFitRect(b, a);
    if (b)
    {
        g_rt->DrawBitmap(b, &d, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nullptr);
    }
    return d;
}
