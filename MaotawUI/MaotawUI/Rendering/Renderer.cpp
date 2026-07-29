#include "../Core/App.h"


HRESULT App::CreateFactories()
{
    HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory_);
    if (FAILED(result))
        return result;

    result = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&writeFactory_));
    if (FAILED(result))
        return result;

    auto createFont = [&](float size, DWRITE_FONT_WEIGHT weight, const wchar_t* family, IDWriteTextFormat** output)
        {
            HRESULT hr = writeFactory_->CreateTextFormat(
                family,
                nullptr,
                weight,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size,
                L"en-US",
                output);

            if (SUCCEEDED(hr))
            {
                (*output)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                (*output)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            }
            return hr;
        };

    if (FAILED(result = createFont(11.2f, DWRITE_FONT_WEIGHT_NORMAL, L"Segoe UI", &text_)))
        return result;
    if (FAILED(result = createFont(9.6f, DWRITE_FONT_WEIGHT_NORMAL, L"Segoe UI", &small_)))
        return result;
    if (FAILED(result = createFont(8.4f, DWRITE_FONT_WEIGHT_NORMAL, L"Segoe UI", &tiny_)))
        return result;
    if (FAILED(result = createFont(12.6f, DWRITE_FONT_WEIGHT_BOLD, L"Segoe UI", &damageText_)))
        return result;
    return createFont(15.0f, DWRITE_FONT_WEIGHT_NORMAL, L"Segoe MDL2 Assets", &icon_);
}

HRESULT App::CreateDeviceResources()
{
    if (renderTarget_)
        return S_OK;

    RECT client{};
    GetClientRect(hwnd_, &client);

    HRESULT result = d2dFactory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_HARDWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        D2D1::HwndRenderTargetProperties(
            hwnd_,
            D2D1::SizeU(client.right - client.left, client.bottom - client.top),
            D2D1_PRESENT_OPTIONS_NONE),
        &renderTarget_);

    if (FAILED(result))
        return result;

    return renderTarget_->CreateSolidColorBrush(Theme::White, &brush_);
}

void App::DiscardDeviceResources()
{
    SafeRelease(brush_);
    SafeRelease(renderTarget_);
}

void App::SetBrush(const D2D1_COLOR_F& color)
{
    brush_->SetColor(color);
}

void App::FillRect(float left, float top, float right, float bottom, const D2D1_COLOR_F& color)
{
    SetBrush(color);
    renderTarget_->FillRectangle(D2D1::RectF(left, top, right, bottom), brush_);
}

void App::DrawRect(float left, float top, float right, float bottom, const D2D1_COLOR_F& color, float stroke)
{
    SetBrush(color);
    renderTarget_->DrawRectangle(D2D1::RectF(left, top, right, bottom), brush_, stroke);
}

void App::FillRound(float left, float top, float right, float bottom, float radius, const D2D1_COLOR_F& color)
{
    SetBrush(color);
    renderTarget_->FillRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radius, radius),
        brush_);
}

void App::DrawRound(float left, float top, float right, float bottom, float radius,
    const D2D1_COLOR_F& color, float stroke)
{
    SetBrush(color);
    renderTarget_->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radius, radius),
        brush_,
        stroke);
}

void App::DrawLine(float x1, float y1, float x2, float y2, const D2D1_COLOR_F& color, float stroke)
{
    SetBrush(color);
    renderTarget_->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), brush_, stroke);
}

void App::DrawText(const std::wstring& value, float left, float top, float right, float bottom,
    const D2D1_COLOR_F& color, IDWriteTextFormat* format, DWRITE_TEXT_ALIGNMENT alignment)
{
    if (!format)
        format = text_;

    format->SetTextAlignment(alignment);
    SetBrush(color);
    renderTarget_->DrawTextW(
        value.c_str(),
        static_cast<UINT32>(value.size()),
        format,
        D2D1::RectF(left, top, right, bottom),
        brush_,
        D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void App::DrawIcon(const wchar_t* glyph, float left, float top, float right, float bottom,
    const D2D1_COLOR_F& color)
{
    DrawText(glyph, left, top, right, bottom, color, icon_, DWRITE_TEXT_ALIGNMENT_CENTER);
}

bool App::Hover(float left, float top, float right, float bottom) const
{
    return Hit(static_cast<float>(mouse_.x), static_cast<float>(mouse_.y), left, top, right, bottom);
}

void App::Divider(float left, float right, float y)
{
    DrawLine(left, y + 0.5f, right, y + 0.5f, Theme::LineSoft, 1.0f);
}
