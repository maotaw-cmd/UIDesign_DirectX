#include "App.h"

bool RebuildOutputBitmapFromPixels()
{
    Release(&g_outputBitmap);

    if (!g_rt || g_outputPixels.empty() || g_outputPixelWidth == 0 || g_outputPixelHeight == 0)
    {
        return false;
    }

    D2D1_BITMAP_PROPERTIES bp = {};
    bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bp.dpiX = 96.0f;
    bp.dpiY = 96.0f;

    D2D1_SIZE_U sz = { g_outputPixelWidth, g_outputPixelHeight };
    return SUCCEEDED(g_rt->CreateBitmap(sz, g_outputPixels.data(), g_outputStride, &bp, &g_outputBitmap));
}

bool LoadEditableOutput(const std::wstring& path)
{
    if (!g_wic || !g_rt)
    {
        return false;
    }

    IWICBitmapDecoder* d = nullptr;
    IWICBitmapFrameDecode* f = nullptr;
    IWICFormatConverter* c = nullptr;
    bool ok = false;

    Release(&g_outputBitmap);
    g_outputPixels.clear();
    g_outputPixelWidth = 0;
    g_outputPixelHeight = 0;
    g_outputStride = 0;

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

    UINT w = 0;
    UINT h = 0;

    if (SUCCEEDED(hr))
    {
        hr = f->GetSize(&w, &h);
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
        g_outputPixelWidth = w;
        g_outputPixelHeight = h;
        g_outputStride = w * 4;
        g_outputPixels.resize(static_cast<size_t>(g_outputStride) * h);

        hr = c->CopyPixels(
            nullptr,
            g_outputStride,
            static_cast<UINT>(g_outputPixels.size()),
            g_outputPixels.data());
    }

    if (SUCCEEDED(hr))
    {
        ok = RebuildOutputBitmapFromPixels();
    }

    Release(&c);
    Release(&f);
    Release(&d);
    return ok;
}

bool SaveEditedOutputPng(const std::wstring& path)
{
    if (g_outputPixels.empty() || !g_wic)
    {
        return false;
    }

    IWICStream* stream = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* bag = nullptr;

    HRESULT hr = g_wic->CreateStream(&stream);
    if (SUCCEEDED(hr))
    {
        hr = stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
    }
    if (SUCCEEDED(hr))
    {
        hr = g_wic->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc);
    }
    if (SUCCEEDED(hr))
    {
        hr = enc->Initialize(stream, WICBitmapEncoderNoCache);
    }
    if (SUCCEEDED(hr))
    {
        hr = enc->CreateNewFrame(&frame, &bag);
    }
    if (SUCCEEDED(hr))
    {
        hr = frame->Initialize(bag);
    }
    if (SUCCEEDED(hr))
    {
        hr = frame->SetSize(g_outputPixelWidth, g_outputPixelHeight);
    }

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppPBGRA;
    if (SUCCEEDED(hr))
    {
        hr = frame->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr))
    {
        hr = frame->WritePixels(
            g_outputPixelHeight,
            g_outputStride,
            static_cast<UINT>(g_outputPixels.size()),
            g_outputPixels.data());
    }
    if (SUCCEEDED(hr))
    {
        hr = frame->Commit();
    }
    if (SUCCEEDED(hr))
    {
        hr = enc->Commit();
    }

    Release(&frame);
    Release(&enc);
    Release(&stream);
    if (bag)
    {
        bag->Release();
    }

    return SUCCEEDED(hr);
}

void UpdateFeatherFromX(float x, float width)
{
    D2D1_RECT_F tr = FeatherTrackRect(width);
    float t = (x - tr.left) / (tr.right - tr.left);
    g_featherValue = std::clamp(t, 0.0f, 1.0f);
}

bool RefineSpotAtPoint(float x, float y, float width)
{
    if (!g_outputBitmap || g_outputPixels.empty())
    {
        return false;
    }

    D2D1_RECT_F view = CurrentPreviewImageRect(width);
    if (!Inside(x, y, view))
    {
        return false;
    }

    const float sx = static_cast<float>(g_outputPixelWidth) / (view.right - view.left);
    const float sy = static_cast<float>(g_outputPixelHeight) / (view.bottom - view.top);
    const float ix = (x - view.left) * sx;
    const float iy = (y - view.top) * sy;

    const int cx = static_cast<int>((std::max)(0.0f, (std::min)(static_cast<float>(g_outputPixelWidth) - 1.0f, ix)));
    const int cy = static_cast<int>((std::max)(0.0f, (std::min)(static_cast<float>(g_outputPixelHeight) - 1.0f, iy)));

    const size_t sampleIndex = static_cast<size_t>(cy) * g_outputStride + static_cast<size_t>(cx) * 4;
    const float sampleB = static_cast<float>(g_outputPixels[sampleIndex]);
    const float sampleG = static_cast<float>(g_outputPixels[sampleIndex + 1]);
    const float sampleR = static_cast<float>(g_outputPixels[sampleIndex + 2]);
    const float sampleA = static_cast<float>(g_outputPixels[sampleIndex + 3]);

    const float scale = (sx + sy) * 0.5f;
    const float radius = 52.0f * scale;
    const float soft = 0.12f + g_featherValue * 0.78f;
    const float inner = radius * (1.0f - soft);
    const float tolerance = 34.0f + g_featherValue * 72.0f;

    const int minX = static_cast<int>((std::max)(0.0f, static_cast<float>(std::floor(ix - radius))));
    const int maxX = static_cast<int>((std::min)(static_cast<float>(g_outputPixelWidth) - 1.0f, static_cast<float>(std::ceil(ix + radius))));
    const int minY = static_cast<int>((std::max)(0.0f, static_cast<float>(std::floor(iy - radius))));
    const int maxY = static_cast<int>((std::min)(static_cast<float>(g_outputPixelHeight) - 1.0f, static_cast<float>(std::ceil(iy + radius))));

    bool changed = false;

    for (int py = minY; py <= maxY; ++py)
    {
        for (int px = minX; px <= maxX; ++px)
        {
            const float dx = px + 0.5f - ix;
            const float dy = py + 0.5f - iy;
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > radius)
            {
                continue;
            }

            const size_t idx = static_cast<size_t>(py) * g_outputStride + static_cast<size_t>(px) * 4;
            const float b = static_cast<float>(g_outputPixels[idx]);
            const float g = static_cast<float>(g_outputPixels[idx + 1]);
            const float r = static_cast<float>(g_outputPixels[idx + 2]);
            const float a = static_cast<float>(g_outputPixels[idx + 3]);

            const float db = b - sampleB;
            const float dg = g - sampleG;
            const float dr = r - sampleR;
            const float colourDistance = std::sqrt(db * db + dg * dg + dr * dr);
            const float alphaDistance = std::fabs(a - sampleA) * 0.32f;
            const float similarity = 1.0f - (colourDistance + alphaDistance) / tolerance;

            if (similarity <= 0.0f)
            {
                continue;
            }

            const float spatial =
                (dist <= inner)
                    ? 1.0f
                    : (1.0f - (dist - inner) / (std::max)(1.0f, radius - inner));

            const float amount = std::clamp(similarity * spatial, 0.0f, 1.0f);
            if (amount < 0.04f)
            {
                continue;
            }

            const float keep = 1.0f - amount;
            g_outputPixels[idx] = static_cast<BYTE>(std::clamp(static_cast<int>(std::lround(b * keep)), 0, 255));
            g_outputPixels[idx + 1] = static_cast<BYTE>(std::clamp(static_cast<int>(std::lround(g * keep)), 0, 255));
            g_outputPixels[idx + 2] = static_cast<BYTE>(std::clamp(static_cast<int>(std::lround(r * keep)), 0, 255));
            g_outputPixels[idx + 3] = static_cast<BYTE>(std::clamp(static_cast<int>(std::lround(a * keep)), 0, 255));
            changed = true;
        }
    }

    if (changed)
    {
        RebuildOutputBitmapFromPixels();
    }

    return changed;
}
