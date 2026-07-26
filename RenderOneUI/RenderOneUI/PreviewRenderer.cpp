#include "PreviewRenderer.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <random>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

template <typename T>
static T MaxValue(T a, T b)
{
    return (a > b) ? a : b;
}

template <typename T>
static T MinValue(T a, T b)
{
    return (a < b) ? a : b;
}
struct Vertex { XMFLOAT3 position; XMFLOAT3 normal; };
#include "EmbeddedCharacterMesh.h"

namespace PreviewRenderer
{
namespace
{
    struct alignas(16) Constants
    {
        XMFLOAT4X4 worldViewProjection;
        XMFLOAT4X4 world;
        XMFLOAT4 colour;
        float outlinePass;
        float outlineWidthPixels;
        float viewportWidth;
        float viewportHeight;
    };
    static_assert(sizeof(Constants) % 16 == 0, "Constants must be 16-byte aligned");

    struct GlowLayer { float widthMultiplier; float alpha; };
    static constexpr std::array<GlowLayer, 7> glowLayers{{
        { 4.20f, 0.035f }, { 3.45f, 0.055f }, { 2.80f, 0.085f },
        { 2.20f, 0.130f }, { 1.68f, 0.210f }, { 1.35f, 0.380f },
        { 1.00f, 0.980f }
    }};
    struct Particle { float x, y, vx, vy, size, phase; };

    HWND g_window = nullptr;
    float g_displayHealth = 100.0f;
    float g_targetHealth = 100.0f;
    float g_nextDamageAt = 1.0f;
    int g_lastDamage = 0;
    float g_damageStartedAt = -10.0f;
    std::mt19937 g_rng{ 0x4D414F54u };
    UINT g_width = 260, g_height = 470;
    Settings g_settings{};
    float g_time = 0.0f;
    std::vector<Particle> g_particles;

    ComPtr<ID3D11Device> g_device;
    ComPtr<ID3D11DeviceContext> g_context;
    ComPtr<IDXGISwapChain> g_swapChain;
    ComPtr<ID3D11RenderTargetView> g_renderTarget;
    ComPtr<ID3D11Texture2D> g_depthTexture;
    ComPtr<ID3D11DepthStencilView> g_depthView;
    ComPtr<ID3D11VertexShader> g_vertexShader;
    ComPtr<ID3D11PixelShader> g_pixelShader;
    ComPtr<ID3D11InputLayout> g_inputLayout;
    ComPtr<ID3D11Buffer> g_vertexBuffer, g_indexBuffer, g_constantBuffer;
    ComPtr<ID3D11RasterizerState> g_rasterizer;
    ComPtr<ID3D11RasterizerState> g_outlineRasterizer;
    ComPtr<ID3D11DepthStencilState> g_depthState;
    ComPtr<ID3D11DepthStencilState> g_outlineDepthState;
    ComPtr<ID3D11BlendState> g_alphaBlend;
    ComPtr<ID3D11BlendState> g_colourWriteDisabledBlend;

    ComPtr<ID2D1Factory1> g_d2dFactory;
    ComPtr<ID2D1Device> g_d2dDevice;
    ComPtr<ID2D1DeviceContext> g_d2dContext;
    ComPtr<ID2D1Bitmap1> g_d2dTarget;
    ComPtr<ID2D1SolidColorBrush> g_brush;
    ComPtr<IDWriteFactory> g_writeFactory;
    ComPtr<IDWriteTextFormat> g_text, g_small, g_bold;

    D2D1_COLOR_F Colour(const float c[4], float alpha = -1.0f)
    {
        return D2D1::ColorF(c[0], c[1], c[2], alpha < 0.0f ? c[3] : alpha);
    }

    HRESULT Compile(const char* source, const char* entry, const char* target, ComPtr<ID3DBlob>& blob)
    {
        ComPtr<ID3DBlob> errors;
        return D3DCompile(source, std::strlen(source), nullptr, nullptr, nullptr,
            entry, target, D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &errors);
    }

    bool CreateD2DTarget()
    {
        g_d2dTarget.Reset();
        ComPtr<IDXGISurface> surface;
        if (FAILED(g_swapChain->GetBuffer(0, IID_PPV_ARGS(&surface)))) return false;
        const D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96.0f, 96.0f);
        if (FAILED(g_d2dContext->CreateBitmapFromDxgiSurface(surface.Get(), &props, &g_d2dTarget))) return false;
        g_d2dContext->SetTarget(g_d2dTarget.Get());
        return true;
    }

    bool CreateSizeResources()
    {
        if (!g_swapChain) return false;
        g_d2dContext->SetTarget(nullptr);
        g_d2dTarget.Reset(); g_renderTarget.Reset(); g_depthView.Reset(); g_depthTexture.Reset();
        RECT rc{}; GetClientRect(g_window, &rc);
        g_width = MaxValue<UINT>(1u, static_cast<UINT>(rc.right - rc.left));
        g_height = MaxValue<UINT>(1u, static_cast<UINT>(rc.bottom - rc.top));
        if (FAILED(g_swapChain->ResizeBuffers(0, g_width, g_height, DXGI_FORMAT_UNKNOWN, 0))) return false;
        ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;
        if (FAILED(g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &g_renderTarget))) return false;
        D3D11_TEXTURE2D_DESC dd{}; dd.Width=g_width; dd.Height=g_height; dd.MipLevels=1; dd.ArraySize=1;
        dd.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; dd.SampleDesc.Count=1; dd.BindFlags=D3D11_BIND_DEPTH_STENCIL;
        if (FAILED(g_device->CreateTexture2D(&dd, nullptr, &g_depthTexture))) return false;
        if (FAILED(g_device->CreateDepthStencilView(g_depthTexture.Get(), nullptr, &g_depthView))) return false;
        return CreateD2DTarget();
    }

    void Line(float x1,float y1,float x2,float y2,D2D1_COLOR_F c,float width=1.0f)
    { g_brush->SetColor(c); g_d2dContext->DrawLine({x1,y1},{x2,y2},g_brush.Get(),width); }
    void Rect(float l,float t,float r,float b,D2D1_COLOR_F c,float width=1.0f)
    { g_brush->SetColor(c); g_d2dContext->DrawRectangle({l,t,r,b},g_brush.Get(),width); }
    void Round(float l,float t,float r,float b,float radius,D2D1_COLOR_F c,float width=1.0f)
    { g_brush->SetColor(c); g_d2dContext->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(l,t,r,b),radius,radius),g_brush.Get(),width); }
    void Fill(float l,float t,float r,float b,D2D1_COLOR_F c)
    { g_brush->SetColor(c); g_d2dContext->FillRectangle({l,t,r,b},g_brush.Get()); }
    void Text(const wchar_t* s,float l,float t,float r,float b,D2D1_COLOR_F c,IDWriteTextFormat* f,DWRITE_TEXT_ALIGNMENT align)
    {
        f->SetTextAlignment(align); g_brush->SetColor(c);
        g_d2dContext->DrawTextW(s, static_cast<UINT32>(wcslen(s)), f, {l,t,r,b}, g_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }
    void CornerBox(float l,float t,float r,float b,D2D1_COLOR_F c)
    {
        const float x=(r-l)*0.25f, y=(b-t)*0.18f;
        Line(l,t,l+x,t,c,2.0f); Line(l,t,l,t+y,c,2.0f);
        Line(r-x,t,r,t,c,2.0f); Line(r,t,r,t+y,c,2.0f);
        Line(l,b-y,l,b,c,2.0f); Line(l,b,l+x,b,c,2.0f);
        Line(r,b-y,r,b,c,2.0f); Line(r-x,b,r,b,c,2.0f);
    }

    void EnsureParticles()
    {
        const int wanted = std::clamp(static_cast<int>(g_settings.particleAmount), 0, 90);
        while (static_cast<int>(g_particles.size()) < wanted)
        {
            const int i = static_cast<int>(g_particles.size()) + 1;
            Particle p{};
            p.x = std::fmod(i * 73.0f, static_cast<float>(MaxValue<UINT>(1u, g_width)));
            p.y = std::fmod(i * 113.0f, static_cast<float>(MaxValue<UINT>(1u, g_height)));
            p.vx = ((i % 5) - 2) * 0.08f; p.vy = 0.16f + (i % 7) * 0.025f;
            p.size = 0.8f + (i % 3) * 0.45f; p.phase = i * 0.7f;
            g_particles.push_back(p);
        }
        if (static_cast<int>(g_particles.size()) > wanted) g_particles.resize(wanted);
    }

    void DrawGlassOutline()
    {
        const float w = static_cast<float>(g_width);
        const float h = static_cast<float>(g_height);
        const float inset = 1.0f;
        const float radius = 9.0f;

        Round(inset + 1.0f, inset + 1.0f, w - inset - 1.0f, h - inset - 1.0f,
            radius - 1.0f, D2D1::ColorF(0.02f, 0.027f, 0.039f, 0.82f), 1.0f);
        Round(inset, inset, w - inset, h - inset,
            radius, D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.34f), 1.0f);
        Round(inset + 2.0f, inset + 2.0f, w - inset - 2.0f, h - inset - 2.0f,
            radius - 2.0f, D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.075f), 1.0f);

        Line(14.0f, 2.0f, w - 16.0f, 2.0f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.20f), 1.0f);
        Line(15.0f, 1.8f, 48.0f, 1.8f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.78f), 1.35f);
        Line(w * 0.43f, 1.8f, w * 0.50f, 1.8f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.48f), 1.15f);
        Line(w - 86.0f, 1.8f, w - 42.0f, 1.8f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.64f), 1.25f);
        Line(1.8f, 15.0f, 1.8f, 47.0f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.46f), 1.0f);
        Line(w - 1.8f, 16.0f, w - 1.8f, 39.0f,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.20f), 1.0f);
    }

    void DrawOverlay()
    {
        g_d2dContext->BeginDraw();
        g_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

        if (g_settings.particles)
        {
            EnsureParticles();
            const float speed = MaxValue(1.0f, g_settings.particleSpeed) / 22.0f;
            for (auto& p : g_particles)
            {
                p.x += p.vx * speed; p.y += p.vy * speed;
                if (p.y > g_height + 4.0f) p.y = -4.0f;
                if (p.x < -4.0f) p.x = g_width + 4.0f; else if (p.x > g_width + 4.0f) p.x = -4.0f;
                const float a = 0.10f + 0.10f * (0.5f + 0.5f * std::sin(g_time + p.phase));
                g_brush->SetColor(Colour(g_settings.accent, a));
                g_d2dContext->FillEllipse(D2D1::Ellipse({p.x,p.y},p.size,p.size),g_brush.Get());
            }
        }

        const float scale = std::clamp(g_settings.characterScale, 0.55f, 1.15f);
        const float boxW = 166.0f * scale, boxH = 382.0f * scale;
        const float cx = g_width * 0.5f, cy = g_height * 0.50f;
        const float l=cx-boxW*0.5f, r=cx+boxW*0.5f, t=cy-boxH*0.5f, b=cy+boxH*0.5f;

        if (g_settings.filled) Fill(l,t,r,b,Colour(g_settings.filledColour,0.14f));
        if (g_settings.box)
        {
            if (g_settings.cornerBox) CornerBox(l,t,r,b,Colour(g_settings.boxColour));
            else Rect(l,t,r,b,Colour(g_settings.boxColour),1.2f);
        }
        if (g_settings.name) Text(L"PLAYER",l-8,t-24,r+8,t-4,D2D1::ColorF(0.95f,0.95f,0.96f,1),g_bold.Get(),DWRITE_TEXT_ALIGNMENT_CENTER);
        // Preview damage simulation: every few seconds take a random 5-35 hit.
        if (g_time >= g_nextDamageAt)
        {
            std::uniform_int_distribution<int> damageDistribution(5, 35);
            std::uniform_real_distribution<float> delayDistribution(1.15f, 2.25f);
            g_lastDamage = damageDistribution(g_rng);
            g_targetHealth = MaxValue(0.0f, g_targetHealth - static_cast<float>(g_lastDamage));
            if (g_targetHealth <= 0.0f)
                g_targetHealth = 100.0f;
            g_damageStartedAt = g_time;
            g_nextDamageAt = g_time + delayDistribution(g_rng);
        }
        const float healthLerp = 1.0f - std::exp(-4.8f * (1.0f / 60.0f));
        g_displayHealth += (g_targetHealth - g_displayHealth) * healthLerp;

        if (g_settings.healthBar)
        {
            const float barWidth = std::clamp(g_settings.healthBarWidth, 6.0f, 20.0f);
            const float barR = l - 8.0f;
            const float barL = barR - barWidth;
            const float ratio = std::clamp(g_displayHealth / 100.0f, 0.0f, 1.0f);
            const float hpTop = b-(b-t)*ratio;
            const D2D1_COLOR_F healthBackColour = Colour(g_settings.healthBackColour, 0.96f);
            D2D1_COLOR_F healthColour = Colour(g_settings.healthColour);
            if (g_displayHealth <= 24.0f)
                healthColour = D2D1::ColorF(0.94f, 0.16f, 0.18f, 1.0f);
            else if (g_displayHealth <= 40.0f)
                healthColour = D2D1::ColorF(1.0f, 0.48f, 0.08f, 1.0f);

            if (g_settings.healthStyle == 1)
            {
                // Draw both the background and foreground as matching segments.
                // This keeps the gaps visible when the background colour changes.
                const int segments = 10;
                for (int i = 0; i < segments; ++i)
                {
                    const float segB = b - (b - t) * static_cast<float>(i) / static_cast<float>(segments);
                    const float segT = b - (b - t) * static_cast<float>(i + 1) / static_cast<float>(segments) + 2.5f;
                    const float cellTop = segT;
                    const float cellBottom = segB - 1.0f;

                    Fill(barL + 1.5f, cellTop, barR - 1.5f, cellBottom, healthBackColour);

                    if (cellBottom > hpTop)
                    {
                        Fill(
                            barL + 1.5f,
                            MaxValue(cellTop, hpTop),
                            barR - 1.5f,
                            cellBottom,
                            healthColour);
                    }
                }
            }
            else
            {
                Fill(barL, t, barR, b, healthBackColour);
                Fill(barL + 1.5f, hpTop, barR - 1.5f, b - 1.5f, healthColour);
            }
            const std::wstring healthText = std::to_wstring(static_cast<int>(std::round(g_displayHealth)));
            Text(healthText.c_str(),barL-31.0f,hpTop-9,barL-4.0f,hpTop+11,D2D1::ColorF(0.92f,0.94f,0.97f,1),g_bold.Get(),DWRITE_TEXT_ALIGNMENT_TRAILING);
        }
        if (g_settings.distance) Text(L"42 m",l,b+5,r,b+22,D2D1::ColorF(0.72f,0.75f,0.79f,1),g_small.Get(),DWRITE_TEXT_ALIGNMENT_CENTER);
        if (g_settings.weapon) Text(L"RIFLE",l,b+(g_settings.distance?22.0f:5.0f),r,b+(g_settings.distance?40.0f:23.0f),D2D1::ColorF(0.86f,0.87f,0.9f,1),g_small.Get(),DWRITE_TEXT_ALIGNMENT_CENTER);
        if (g_settings.damage && g_lastDamage > 0)
        {
            const float phase=std::clamp((g_time-g_damageStartedAt)/1.15f,0.0f,1.0f);
            const float smooth=phase*phase*(3.0f-2.0f*phase);
            const float rise=smooth*42.0f;
            const float alpha=std::clamp(1.0f-smooth,0.0f,1.0f);
            const std::wstring damageText = L"-" + std::to_wstring(g_lastDamage);
            Text(damageText.c_str(),r-5,t+92-rise,r+58,t+126-rise,Colour(g_settings.damageColour,alpha),g_bold.Get(),DWRITE_TEXT_ALIGNMENT_LEADING);
        }
        if (g_settings.snapline) Line(g_width*0.5f,g_height-4.0f,cx,b,Colour(g_settings.lineColour),1.0f);

        DrawGlassOutline();
        g_d2dContext->EndDraw();
    }
}

void SetSettings(const Settings& settings) { g_settings = settings; }

bool Initialize(HWND window)
{
    g_window=window; RECT rc{}; GetClientRect(window,&rc);
    g_width = static_cast<UINT>(MaxValue<LONG>(1L, rc.right - rc.left));
    g_height = static_cast<UINT>(MaxValue<LONG>(1L, rc.bottom - rc.top));
    DXGI_SWAP_CHAIN_DESC swap{}; swap.BufferCount=2; swap.BufferDesc.Width=g_width; swap.BufferDesc.Height=g_height;
    swap.BufferDesc.Format=DXGI_FORMAT_B8G8R8A8_UNORM; swap.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap.OutputWindow=window; swap.SampleDesc.Count=1; swap.Windowed=TRUE; swap.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL level{};
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,0,D3D11_SDK_VERSION,&swap,&g_swapChain,&g_device,&level,&g_context))) return false;

    D2D1_FACTORY_OPTIONS fo{};
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory1),&fo,
        reinterpret_cast<void**>(g_d2dFactory.GetAddressOf())))) return false;
    ComPtr<IDXGIDevice> dxgiDevice; if (FAILED(g_device.As(&dxgiDevice))) return false;
    if (FAILED(g_d2dFactory->CreateDevice(dxgiDevice.Get(),&g_d2dDevice))) return false;
    if (FAILED(g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,&g_d2dContext))) return false;
    if (FAILED(g_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1,1,1,1),&g_brush))) return false;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),reinterpret_cast<IUnknown**>(g_writeFactory.GetAddressOf())))) return false;
    auto fmt=[&](float size,DWRITE_FONT_WEIGHT weight,ComPtr<IDWriteTextFormat>& out){
        if (FAILED(g_writeFactory->CreateTextFormat(L"Segoe UI",nullptr,weight,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,size,L"en-US",&out))) return false;
        out->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP); out->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); return true; };
    if (!fmt(11,DWRITE_FONT_WEIGHT_NORMAL,g_text)||!fmt(9,DWRITE_FONT_WEIGHT_NORMAL,g_small)||!fmt(11,DWRITE_FONT_WEIGHT_SEMI_BOLD,g_bold)) return false;
    if (!CreateSizeResources()) return false;

    static const char* shader=R"(
cbuffer Constants : register(b0)
{
    matrix worldViewProjection;
    matrix world;
    float4 objectColour;
    float outlinePass;
    float outlineWidthPixels;
    float viewportWidth;
    float viewportHeight;
};
struct VSInput { float3 position : POSITION; float3 normal : NORMAL; };
struct PSInput { float4 position : SV_POSITION; float3 normal : NORMAL; };
PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 clipPosition = mul(float4(input.position, 1.0f), worldViewProjection);
    if (outlinePass > 0.5f && outlineWidthPixels > 0.0f)
    {
        const float normalProbeDistance = 0.05f;
        float4 normalClipPosition = mul(float4(input.position + normalize(input.normal) * normalProbeDistance, 1.0f), worldViewProjection);
        float safeClipW = abs(clipPosition.w) < 0.00001f ? (clipPosition.w < 0.0f ? -0.00001f : 0.00001f) : clipPosition.w;
        float safeNormalW = abs(normalClipPosition.w) < 0.00001f ? (normalClipPosition.w < 0.0f ? -0.00001f : 0.00001f) : normalClipPosition.w;
        float2 positionNdc = clipPosition.xy / safeClipW;
        float2 normalNdc = normalClipPosition.xy / safeNormalW;
        float safeViewportWidth = max(viewportWidth, 1.0f);
        float safeViewportHeight = max(viewportHeight, 1.0f);
        float2 projectedNormalPixels = float2((normalNdc.x-positionNdc.x)*safeViewportWidth*0.5f, -(normalNdc.y-positionNdc.y)*safeViewportHeight*0.5f);
        float projectedLength = length(projectedNormalPixels);
        if (projectedLength > 0.0001f)
        {
            float2 pixelDirection = projectedNormalPixels / projectedLength;
            float2 pixelOffset = pixelDirection * outlineWidthPixels;
            float2 ndcOffset = float2(pixelOffset.x * 2.0f / safeViewportWidth, -pixelOffset.y * 2.0f / safeViewportHeight);
            clipPosition.xy += ndcOffset * clipPosition.w;
        }
    }
    output.position = clipPosition;
    output.normal = normalize(mul(float4(input.normal,0),world).xyz);
    return output;
}
float4 PSMain(PSInput input) : SV_TARGET
{
    if (outlinePass > 0.5f) return objectColour;
    float3 n=normalize(input.normal); float3 ld=normalize(float3(-.45,.75,-.55));
    float d=saturate(dot(n,ld)); float rim=pow(1-saturate(dot(n,float3(0,0,-1))),2.2);
    float3 c=objectColour.rgb*(.18+d*.82); c+=float3(.20,.035,.045)*rim;
    return float4(c,1);
})";
    ComPtr<ID3DBlob> vs,ps; if(FAILED(Compile(shader,"VSMain","vs_4_0",vs))||FAILED(Compile(shader,"PSMain","ps_4_0",ps)))return false;
    if(FAILED(g_device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&g_vertexShader))||FAILED(g_device->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&g_pixelShader)))return false;
    const D3D11_INPUT_ELEMENT_DESC il[]={{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,offsetof(Vertex,position),D3D11_INPUT_PER_VERTEX_DATA,0},{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,offsetof(Vertex,normal),D3D11_INPUT_PER_VERTEX_DATA,0}};
    if(FAILED(g_device->CreateInputLayout(il,2,vs->GetBufferPointer(),vs->GetBufferSize(),&g_inputLayout)))return false;
    D3D11_BUFFER_DESC vb{};vb.ByteWidth=sizeof(vertices);vb.Usage=D3D11_USAGE_IMMUTABLE;vb.BindFlags=D3D11_BIND_VERTEX_BUFFER;D3D11_SUBRESOURCE_DATA vd{};vd.pSysMem=vertices;if(FAILED(g_device->CreateBuffer(&vb,&vd,&g_vertexBuffer)))return false;
    D3D11_BUFFER_DESC ib{};ib.ByteWidth=sizeof(indices);ib.Usage=D3D11_USAGE_IMMUTABLE;ib.BindFlags=D3D11_BIND_INDEX_BUFFER;D3D11_SUBRESOURCE_DATA id{};id.pSysMem=indices;if(FAILED(g_device->CreateBuffer(&ib,&id,&g_indexBuffer)))return false;
    D3D11_BUFFER_DESC cb{};cb.ByteWidth=sizeof(Constants);cb.Usage=D3D11_USAGE_DYNAMIC;cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER;cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;if(FAILED(g_device->CreateBuffer(&cb,nullptr,&g_constantBuffer)))return false;
    D3D11_RASTERIZER_DESC rs{};
    rs.FillMode=D3D11_FILL_SOLID; rs.CullMode=D3D11_CULL_BACK; rs.FrontCounterClockwise=FALSE;
    rs.DepthClipEnable=TRUE; rs.ScissorEnable=TRUE;
    if(FAILED(g_device->CreateRasterizerState(&rs,&g_rasterizer)))return false;
    D3D11_RASTERIZER_DESC outlineRs=rs; outlineRs.CullMode=D3D11_CULL_FRONT;
    if(FAILED(g_device->CreateRasterizerState(&outlineRs,&g_outlineRasterizer)))return false;

    D3D11_DEPTH_STENCIL_DESC ds{}; ds.DepthEnable=TRUE; ds.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL; ds.DepthFunc=D3D11_COMPARISON_LESS_EQUAL;
    if(FAILED(g_device->CreateDepthStencilState(&ds,&g_depthState)))return false;
    D3D11_DEPTH_STENCIL_DESC outlineDs=ds; outlineDs.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
    if(FAILED(g_device->CreateDepthStencilState(&outlineDs,&g_outlineDepthState)))return false;

    D3D11_BLEND_DESC blend{}; blend.RenderTarget[0].BlendEnable=TRUE;
    blend.RenderTarget[0].SrcBlend=D3D11_BLEND_SRC_ALPHA; blend.RenderTarget[0].DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
    blend.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD; blend.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
    blend.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA; blend.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
    blend.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    if(FAILED(g_device->CreateBlendState(&blend,&g_alphaBlend)))return false;
    D3D11_BLEND_DESC depthOnly{}; depthOnly.RenderTarget[0].BlendEnable=FALSE; depthOnly.RenderTarget[0].RenderTargetWriteMask=0;
    if(FAILED(g_device->CreateBlendState(&depthOnly,&g_colourWriteDisabledBlend)))return false;
    SetTimer(window,2,16,nullptr); return true;
}

void Render()
{
    if(!g_context||!g_renderTarget)return;
    g_time+=0.016f;
    const float clear[4]={g_settings.backgroundColour[0],g_settings.backgroundColour[1],g_settings.backgroundColour[2],1.0f};
    g_context->OMSetRenderTargets(1,g_renderTarget.GetAddressOf(),g_depthView.Get());
    g_context->ClearRenderTargetView(g_renderTarget.Get(),clear);
    g_context->ClearDepthStencilView(g_depthView.Get(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1,0);

    D3D11_VIEWPORT vp{}; vp.Width=(float)g_width; vp.Height=(float)g_height; vp.MaxDepth=1;
    g_context->RSSetViewports(1,&vp);
    D3D11_RECT scissor{0,0,static_cast<LONG>(g_width),static_cast<LONG>(g_height)};
    g_context->RSSetScissorRects(1,&scissor);

    UINT stride=sizeof(Vertex),offset=0;
    g_context->IASetInputLayout(g_inputLayout.Get());
    g_context->IASetVertexBuffers(0,1,g_vertexBuffer.GetAddressOf(),&stride,&offset);
    g_context->IASetIndexBuffer(g_indexBuffer.Get(),DXGI_FORMAT_R32_UINT,0);
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_context->VSSetShader(g_vertexShader.Get(),nullptr,0);
    g_context->PSSetShader(g_pixelShader.Get(),nullptr,0);
    g_context->VSSetConstantBuffers(0,1,g_constantBuffer.GetAddressOf());
    g_context->PSSetConstantBuffers(0,1,g_constantBuffer.GetAddressOf());

    const float aspect=(float)g_width/(float)g_height;
    const float scale=std::clamp(g_settings.characterScale,0.55f,1.15f);
    const float rotation=g_time*0.55f;
    const XMMATRIX world=XMMatrixScaling(scale,scale,scale)*XMMatrixRotationY(rotation)*XMMatrixTranslation(0,0.0f,0);
    const XMMATRIX view=XMMatrixLookAtLH(XMVectorSet(0,.94f,-4.85f,1),XMVectorSet(0,.94f,0,1),XMVectorSet(0,1,0,0));
    const XMMATRIX proj=XMMatrixPerspectiveFovLH(XMConvertToRadians(33),aspect,.05f,100);

    Constants constants{};
    XMStoreFloat4x4(&constants.worldViewProjection,XMMatrixTranspose(world*view*proj));
    XMStoreFloat4x4(&constants.world,XMMatrixTranspose(world));
    constants.viewportWidth=static_cast<float>(g_width);
    constants.viewportHeight=static_cast<float>(g_height);

    auto drawMesh=[&](bool outline,float widthPixels,const XMFLOAT4& colour)
    {
        constants.outlinePass=outline?1.0f:0.0f;
        constants.outlineWidthPixels=widthPixels;
        constants.colour=colour;
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if(SUCCEEDED(g_context->Map(g_constantBuffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))
        {
            std::memcpy(mapped.pData,&constants,sizeof(constants));
            g_context->Unmap(g_constantBuffer.Get(),0);
            g_context->DrawIndexed(kEmbeddedIndexCount,0,0);
        }
    };

    const float blendFactor[4]={0,0,0,0};
    if(g_settings.visualGlow)
    {
        // 1) Depth-only normal mesh pre-pass.
        g_context->RSSetState(g_rasterizer.Get());
        g_context->OMSetDepthStencilState(g_depthState.Get(),0);
        g_context->OMSetBlendState(g_colourWriteDisabledBlend.Get(),blendFactor,0xFFFFFFFFu);
        drawMesh(false,0.0f,XMFLOAT4(.63f,.66f,.72f,1.0f));

        // 2) Screen-space inverted-hull layers, widest/weakest first.
        g_context->RSSetState(g_outlineRasterizer.Get());
        g_context->OMSetDepthStencilState(g_outlineDepthState.Get(),0);
        g_context->OMSetBlendState(g_alphaBlend.Get(),blendFactor,0xFFFFFFFFu);
        const float visibleThickness=std::clamp(g_settings.visualGlowThickness,0.5f,6.0f);
        for(const GlowLayer& layer:glowLayers)
        {
            drawMesh(true,visibleThickness*layer.widthMultiplier,
                XMFLOAT4(g_settings.visualGlowColour[0],g_settings.visualGlowColour[1],g_settings.visualGlowColour[2],layer.alpha));
        }
    }

    // 3) Normal lit character last, covering all interior outline fragments.
    g_context->RSSetState(g_rasterizer.Get());
    g_context->OMSetDepthStencilState(g_depthState.Get(),0);
    g_context->OMSetBlendState(g_alphaBlend.Get(),blendFactor,0xFFFFFFFFu);
    drawMesh(false,0.0f,XMFLOAT4(.63f,.66f,.72f,1.0f));
    g_context->OMSetBlendState(nullptr,blendFactor,0xFFFFFFFFu);

    DrawOverlay();
    g_swapChain->Present(1,0);
}

void Shutdown()
{
    if(g_window)KillTimer(g_window,2);g_d2dContext->SetTarget(nullptr);g_bold.Reset();g_small.Reset();g_text.Reset();g_writeFactory.Reset();g_brush.Reset();g_d2dTarget.Reset();g_d2dContext.Reset();g_d2dDevice.Reset();g_d2dFactory.Reset();g_colourWriteDisabledBlend.Reset();g_alphaBlend.Reset();g_outlineDepthState.Reset();g_depthState.Reset();g_outlineRasterizer.Reset();g_rasterizer.Reset();g_constantBuffer.Reset();g_indexBuffer.Reset();g_vertexBuffer.Reset();g_inputLayout.Reset();g_pixelShader.Reset();g_vertexShader.Reset();g_depthView.Reset();g_depthTexture.Reset();g_renderTarget.Reset();g_swapChain.Reset();g_context.Reset();g_device.Reset();g_window=nullptr;
}

LRESULT HandleMessage(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam,bool& handled)
{
    handled=true;switch(message){case WM_ERASEBKGND:return 1;case WM_TIMER:if(wParam==2){Render();return 0;}break;case WM_PAINT:{PAINTSTRUCT ps{};BeginPaint(hwnd,&ps);Render();EndPaint(hwnd,&ps);return 0;}case WM_SIZE:if(g_swapChain&&wParam!=SIZE_MINIMIZED)CreateSizeResources();return 0;case WM_NCHITTEST:return HTCLIENT;case WM_LBUTTONDOWN:case WM_MOUSEMOVE:case WM_LBUTTONUP:return 0;case WM_DESTROY:return 0;default:handled=false;break;}return 0;
}
}
