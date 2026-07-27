#pragma once

    // ------------------------------------------------------------------------
    // Device creation
    // ------------------------------------------------------------------------

    HRESULT CreateTextFormat(
        float size,
        DWRITE_FONT_WEIGHT weight,
        ComPtr<IDWriteTextFormat>& output)
    {
        HRESULT hr = app.writeFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            weight,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-US",
            &output);

        if (SUCCEEDED(hr))
        {
            output->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            output->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        return hr;
    }

    HRESULT CreateFactories()
    {
        D2D1_FACTORY_OPTIONS options{};

#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory1),
            &options,
            reinterpret_cast<void**>(app.d2dFactory.GetAddressOf()));

        if (FAILED(hr))
            return hr;

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(app.writeFactory.GetAddressOf()));

        if (FAILED(hr))
            return hr;

        if (FAILED(hr = CreateTextFormat(12, DWRITE_FONT_WEIGHT_NORMAL, app.text12))) return hr;
        if (FAILED(hr = CreateTextFormat(13, DWRITE_FONT_WEIGHT_NORMAL, app.text13))) return hr;
        if (FAILED(hr = CreateTextFormat(14, DWRITE_FONT_WEIGHT_NORMAL, app.text14))) return hr;
        if (FAILED(hr = CreateTextFormat(16, DWRITE_FONT_WEIGHT_SEMI_BOLD, app.text16Bold))) return hr;
        if (FAILED(hr = CreateTextFormat(19, DWRITE_FONT_WEIGHT_SEMI_BOLD, app.text19Bold))) return hr;
        if (FAILED(hr = CreateTextFormat(27, DWRITE_FONT_WEIGHT_BOLD, app.text27Bold))) return hr;

        hr = app.writeFactory->CreateTextFormat(
            L"Segoe MDL2 Assets", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            15.0f, L"en-US", &app.iconFont);
        if (SUCCEEDED(hr))
        {
            app.iconFont->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            app.iconFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
        return hr;
    }

    HRESULT CreateD3DDeviceAndSwapChain()
    {
        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel{};
        const D3D_FEATURE_LEVEL requestedLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            requestedLevels,
            ARRAYSIZE(requestedLevels),
            D3D11_SDK_VERSION,
            &app.d3dDevice,
            &featureLevel,
            &app.d3dContext);

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                flags,
                requestedLevels + 1,
                ARRAYSIZE(requestedLevels) - 1,
                D3D11_SDK_VERSION,
                &app.d3dDevice,
                &featureLevel,
                &app.d3dContext);
        }

        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIDevice> dxgiDevice;
        hr = app.d3dDevice.As(&dxgiDevice);
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIAdapter> adapter;
        hr = dxgiDevice->GetAdapter(&adapter);
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory2> factory;
        hr = adapter->GetParent(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
            return hr;

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = app.width;
        desc.Height = app.height;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

        hr = factory->CreateSwapChainForHwnd(
            app.d3dDevice.Get(),
            app.hwnd,
            &desc,
            nullptr,
            nullptr,
            &app.swapChain);

        if (FAILED(hr))
            return hr;

        factory->MakeWindowAssociation(app.hwnd, DXGI_MWA_NO_ALT_ENTER);

        hr = app.d2dFactory->CreateDevice(dxgiDevice.Get(), &app.d2dDevice);
        if (FAILED(hr))
            return hr;

        hr = app.d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &app.d2dContext);

        if (FAILED(hr))
            return hr;

        return app.d2dContext->CreateSolidColorBrush(ui.white, &app.brush);
    }

    HRESULT CreateSizeResources()
    {
        app.renderTargetView.Reset();
        app.depthView.Reset();
        app.depthTexture.Reset();
        app.d2dTarget.Reset();

        app.d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
        app.d2dContext->SetTarget(nullptr);

        HRESULT hr = app.swapChain->ResizeBuffers(
            0,
            app.width,
            app.height,
            DXGI_FORMAT_UNKNOWN,
            0);

        if (FAILED(hr))
            return hr;

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = app.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr))
            return hr;

        hr = app.d3dDevice->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            &app.renderTargetView);

        if (FAILED(hr))
            return hr;

        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = app.width;
        depthDesc.Height = app.height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        hr = app.d3dDevice->CreateTexture2D(
            &depthDesc,
            nullptr,
            &app.depthTexture);

        if (FAILED(hr))
            return hr;

        hr = app.d3dDevice->CreateDepthStencilView(
            app.depthTexture.Get(),
            nullptr,
            &app.depthView);

        if (FAILED(hr))
            return hr;

        ComPtr<IDXGISurface> surface;
        hr = backBuffer.As(&surface);
        if (FAILED(hr))
            return hr;

        const float dpi = static_cast<float>(GetDpiForWindow(app.hwnd));

        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET |
                D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE),
                dpi,
                dpi);

        hr = app.d2dContext->CreateBitmapFromDxgiSurface(
            surface.Get(),
            &bitmapProperties,
            &app.d2dTarget);

        if (FAILED(hr))
            return hr;

        app.d2dContext->SetTarget(app.d2dTarget.Get());
        app.d2dContext->SetDpi(dpi, dpi);

        return S_OK;
    }

    HRESULT CompileShader(
        const char* source,
        const char* entry,
        const char* target,
        ComPtr<ID3DBlob>& blob)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DCompile(
            source,
            strlen(source),
            nullptr,
            nullptr,
            nullptr,
            entry,
            target,
            flags,
            0,
            &blob,
            &errors);

        if (FAILED(hr) && errors)
            OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));

        return hr;
    }

    HRESULT Create3DResources()
    {
        static const char* shaderSource = R"(
cbuffer Constants : register(b0)
{
    matrix worldViewProjection;
    matrix world;
    float4 objectColour;

    // These four floats share one 16-byte constant-buffer register.
    float outlinePass;
    float outlineWidthPixels;
    float viewportWidth;
    float viewportHeight;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 clipPosition = mul(float4(input.position, 1.0f), worldViewProjection);

    if (outlinePass > 0.5f && outlineWidthPixels > 0.0f)
    {
        // Project a nearby point along the mesh normal.  The difference between
        // both projected points gives the normal direction in screen pixels.
        // Moving clip.xy by an NDC offset multiplied by clip.w keeps the visible
        // extrusion approximately constant in pixels at every depth.
        const float normalProbeDistance = 0.05f;
        float4 normalClipPosition = mul(
            float4(input.position + normalize(input.normal) * normalProbeDistance, 1.0f),
            worldViewProjection);

        float safeClipW = abs(clipPosition.w) < 0.00001f
            ? (clipPosition.w < 0.0f ? -0.00001f : 0.00001f)
            : clipPosition.w;
        float safeNormalW = abs(normalClipPosition.w) < 0.00001f
            ? (normalClipPosition.w < 0.0f ? -0.00001f : 0.00001f)
            : normalClipPosition.w;
        float2 positionNdc = clipPosition.xy / safeClipW;
        float2 normalNdc = normalClipPosition.xy / safeNormalW;

        float safeViewportWidth = max(viewportWidth, 1.0f);
        float safeViewportHeight = max(viewportHeight, 1.0f);
        float2 projectedNormalPixels = float2(
            (normalNdc.x - positionNdc.x) * safeViewportWidth * 0.5f,
            -(normalNdc.y - positionNdc.y) * safeViewportHeight * 0.5f);

        float projectedLength = length(projectedNormalPixels);
        if (projectedLength > 0.0001f)
        {
            float2 pixelDirection = projectedNormalPixels / projectedLength;
            float2 pixelOffset = pixelDirection * outlineWidthPixels;
            float2 ndcOffset = float2(
                pixelOffset.x * 2.0f / safeViewportWidth,
                -pixelOffset.y * 2.0f / safeViewportHeight);

            clipPosition.xy += ndcOffset * clipPosition.w;
        }
    }

    output.position = clipPosition;
    output.normal = normalize(mul(float4(input.normal, 0.0f), world).xyz);
    output.worldPosition = mul(float4(input.position, 1.0f), world).xyz;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Outline passes are intentionally flat and unlit.  The selected UI colour
    // and per-layer alpha are returned without applying character lighting.
    if (outlinePass > 0.5f)
        return objectColour;

    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(float3(-0.4f, 0.8f, -0.55f));
    float diffuse = saturate(dot(normal, lightDirection));
    float rim = pow(1.0f - saturate(dot(normal, normalize(float3(0.0f, 0.0f, -1.0f)))), 2.4f);

    float3 colour = objectColour.rgb * (0.22f + diffuse * 0.78f);
    colour += objectColour.rgb * rim * 0.45f;

    return float4(colour, objectColour.a);
}
)";

        ComPtr<ID3DBlob> vertexBlob;
        ComPtr<ID3DBlob> pixelBlob;

        HRESULT hr = CompileShader(shaderSource, "VSMain", "vs_4_0", vertexBlob);
        if (FAILED(hr))
            return hr;

        hr = CompileShader(shaderSource, "PSMain", "ps_4_0", pixelBlob);
        if (FAILED(hr))
            return hr;

        hr = app.d3dDevice->CreateVertexShader(
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            nullptr,
            &app.vertexShader);

        if (FAILED(hr))
            return hr;

        hr = app.d3dDevice->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            nullptr,
            &app.pixelShader);

        if (FAILED(hr))
            return hr;

        const D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
              static_cast<UINT>(offsetof(Vertex, position)),
              D3D11_INPUT_PER_VERTEX_DATA, 0 },

            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
              static_cast<UINT>(offsetof(Vertex, normal)),
              D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = app.d3dDevice->CreateInputLayout(
            layout,
            ARRAYSIZE(layout),
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            &app.inputLayout);

        if (FAILED(hr))
            return hr;

        // The embedded character mesh is isolated so UI beginners never need to edit it.
        #include "EmbeddedCharacterMesh.inl"

        D3D11_BUFFER_DESC vertexDesc{};
        vertexDesc.ByteWidth = sizeof(vertices);
        vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData{};
        vertexData.pSysMem = vertices;

        hr = app.d3dDevice->CreateBuffer(
            &vertexDesc,
            &vertexData,
            &app.vertexBuffer);

        if (FAILED(hr))
            return hr;

        D3D11_BUFFER_DESC indexDesc{};
        indexDesc.ByteWidth = sizeof(indices);
        indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData{};
        indexData.pSysMem = indices;

        hr = app.d3dDevice->CreateBuffer(
            &indexDesc,
            &indexData,
            &app.indexBuffer);

        if (FAILED(hr))
            return hr;

        app.embeddedIndexCount = kEmbeddedIndexCount;

        D3D11_BUFFER_DESC constantDesc{};
        constantDesc.ByteWidth = sizeof(ConstantBuffer);
        constantDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = app.d3dDevice->CreateBuffer(
            &constantDesc,
            nullptr,
            &app.constantBuffer);

        if (FAILED(hr))
            return hr;

        D3D11_RASTERIZER_DESC rasterDesc{};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthClipEnable = TRUE;
        rasterDesc.ScissorEnable = TRUE;

        hr = app.d3dDevice->CreateRasterizerState(
            &rasterDesc,
            &app.rasterizerState);

        if (FAILED(hr))
            return hr;

        // Inverted-hull state used for a clean silhouette around the model.
        D3D11_RASTERIZER_DESC outlineRasterDesc = rasterDesc;
        outlineRasterDesc.CullMode = D3D11_CULL_FRONT;

        hr = app.d3dDevice->CreateRasterizerState(
            &outlineRasterDesc,
            &app.outlineRasterizerState);

        if (FAILED(hr))
            return hr;

        D3D11_DEPTH_STENCIL_DESC depthStateDesc{};
        depthStateDesc.DepthEnable = TRUE;
        depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

        hr = app.d3dDevice->CreateDepthStencilState(
            &depthStateDesc,
            &app.depthState);

        if (FAILED(hr))
            return hr;

        // Glow layers depth-test against a depth-only model pre-pass but do not
        // write depth themselves.  This lets every transparent layer blend while
        // preventing back-facing outline geometry from showing through the model.
        D3D11_DEPTH_STENCIL_DESC outlineDepthDesc = depthStateDesc;
        outlineDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

        hr = app.d3dDevice->CreateDepthStencilState(
            &outlineDepthDesc,
            &app.outlineDepthState);

        if (FAILED(hr))
            return hr;

        D3D11_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = app.d3dDevice->CreateBlendState(
            &blendDesc,
            &app.alphaBlend);

        if (FAILED(hr))
            return hr;

        D3D11_BLEND_DESC depthOnlyBlendDesc{};
        depthOnlyBlendDesc.RenderTarget[0].BlendEnable = FALSE;
        depthOnlyBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

        return app.d3dDevice->CreateBlendState(
            &depthOnlyBlendDesc,
            &app.colourWriteDisabledBlend);
    }

    // ------------------------------------------------------------------------
    // 3D rendering
    // ------------------------------------------------------------------------

    XMFLOAT4 ToFloat4(const D2D1_COLOR_F& colour, float alpha = 1.0f)
    {
        return XMFLOAT4(colour.r, colour.g, colour.b, alpha);
    }

    void DrawCubePart(
        const XMMATRIX& viewProjection,
        const XMFLOAT3& position,
        const XMFLOAT3& scale,
        const XMFLOAT3& rotation,
        const D2D1_COLOR_F& colour,
        float alpha,
        bool outlinePass,
        float outlineWidthPixels,
        float viewportWidth,
        float viewportHeight)
    {
        const XMMATRIX world =
            XMMatrixScaling(scale.x, scale.y, scale.z) *
            XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
            XMMatrixTranslation(position.x, position.y, position.z);

        const XMMATRIX wvp = world * viewProjection;

        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(app.d3dContext->Map(
            app.constantBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped)))
        {
            return;
        }

        ConstantBuffer constants{};
        XMStoreFloat4x4(&constants.worldViewProjection, XMMatrixTranspose(wvp));
        XMStoreFloat4x4(&constants.world, XMMatrixTranspose(world));
        constants.colour = ToFloat4(colour, alpha);
        constants.outlinePass = outlinePass ? 1.0f : 0.0f;
        constants.outlineWidthPixels = std::max(0.0f, outlineWidthPixels);
        constants.viewportWidth = std::max(1.0f, viewportWidth);
        constants.viewportHeight = std::max(1.0f, viewportHeight);

        memcpy(mapped.pData, &constants, sizeof(constants));
        app.d3dContext->Unmap(app.constantBuffer.Get(), 0);

        app.d3dContext->DrawIndexed(app.embeddedIndexCount, 0, 0);
    }

    void DrawMannequin(float previewLeft, float previewTop, float previewWidth, float previewHeight)
    {
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = previewLeft;
        viewport.TopLeftY = previewTop;
        viewport.Width = previewWidth;
        viewport.Height = previewHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        app.d3dContext->RSSetViewports(1, &viewport);

        // Expand the preview scissor by the maximum glow radius so the soft
        // outer layers are not clipped at the preview boundary.
        const float requestedGlowThickness =
            app.settings.visualGlow
            ? Clamp(app.settings.visualGlowThickness, 0.5f, 6.0f)
            : 0.0f;
        const LONG glowScissorMargin = static_cast<LONG>(
            std::ceil(requestedGlowThickness * 4.20f + 6.0f));

        D3D11_RECT scissor{};
        scissor.left = std::max<LONG>(
            0, static_cast<LONG>(std::floor(previewLeft)) - glowScissorMargin);
        scissor.top = std::max<LONG>(
            0, static_cast<LONG>(std::floor(previewTop)) - glowScissorMargin);
        scissor.right = std::min<LONG>(
            static_cast<LONG>(app.width),
            static_cast<LONG>(std::ceil(previewLeft + previewWidth)) + glowScissorMargin);
        scissor.bottom = std::min<LONG>(
            static_cast<LONG>(app.height),
            static_cast<LONG>(std::ceil(previewTop + previewHeight)) + glowScissorMargin);
        app.d3dContext->RSSetScissorRects(1, &scissor);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        app.d3dContext->IASetInputLayout(app.inputLayout.Get());
        app.d3dContext->IASetVertexBuffers(
            0, 1, app.vertexBuffer.GetAddressOf(), &stride, &offset);
        app.d3dContext->IASetIndexBuffer(
            app.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        app.d3dContext->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        app.d3dContext->VSSetShader(app.vertexShader.Get(), nullptr, 0);
        app.d3dContext->PSSetShader(app.pixelShader.Get(), nullptr, 0);
        app.d3dContext->VSSetConstantBuffers(
            0, 1, app.constantBuffer.GetAddressOf());
        app.d3dContext->PSSetConstantBuffers(
            0, 1, app.constantBuffer.GetAddressOf());
        app.d3dContext->OMSetDepthStencilState(app.depthState.Get(), 0);

        const float blendFactor[4]{ 0, 0, 0, 0 };
        app.d3dContext->OMSetBlendState(
            app.alphaBlend.Get(), blendFactor, 0xFFFFFFFF);

        const float aspect = previewWidth / previewHeight;

        const XMMATRIX view = XMMatrixLookAtLH(
            XMVectorSet(0.0f, 0.95f, -4.8f, 1.0f),
            XMVectorSet(0.0f, 0.95f, 0.0f, 1.0f),
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

        const XMMATRIX projection = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(34.0f),
            aspect,
            0.05f,
            100.0f);

        constexpr float baseScale = 1.12f;
        constexpr float meshCentreY = 0.95616785f;
        const float scale = baseScale * app.settings.modelScale;

        // Scale around the character's visual centre instead of its feet.
        // This prevents the character from appearing to fall downward when reduced.
        const float anchoredY = -0.14f + meshCentreY * (baseScale - scale);
        const XMFLOAT3 position(0.0f, anchoredY, 0.0f);
        const XMFLOAT3 rotation(0.0f, app.modelYaw, 0.0f);

        const XMMATRIX viewProjection = view * projection;

        // Layered screen-space inverted hull.  Every pass uses the exact embedded
        // character mesh and expands vertices along projected normals, so the
        // thickness stays approximately constant in pixels without rendering an
        // enlarged transparent duplicate character.
        if (app.settings.visualGlow)
        {
            struct GlowLayer
            {
                float widthMultiplier;
                float alpha;
            };

            // Draw from widest/weakest to narrowest/brightest.  Straight-alpha
            // blending accumulates the outer layers into a soft halo, while the
            // final narrow pass creates the sharp neon edge.
            static constexpr std::array<GlowLayer, 7> glowLayers
            { {
                { 4.20f, 0.035f },
                { 3.45f, 0.055f },
                { 2.80f, 0.085f },
                { 2.20f, 0.130f },
                { 1.68f, 0.210f },
                { 1.35f, 0.380f },
                { 1.00f, 0.980f }
            } };

            const float visibleThickness =
                Clamp(app.settings.visualGlowThickness, 0.5f, 6.0f);

            // Depth-only model pre-pass.  It is not visible, but it masks the
            // inverted hull so outline geometry cannot draw through the character.
            app.d3dContext->RSSetState(app.rasterizerState.Get());
            app.d3dContext->OMSetDepthStencilState(app.depthState.Get(), 0);
            app.d3dContext->OMSetBlendState(
                app.colourWriteDisabledBlend.Get(), blendFactor, 0xFFFFFFFF);
            DrawCubePart(
                viewProjection,
                position,
                XMFLOAT3(scale, scale, scale),
                rotation,
                app.settings.modelColour,
                1.0f,
                false,
                0.0f,
                previewWidth,
                previewHeight);

            app.d3dContext->OMSetBlendState(
                app.alphaBlend.Get(), blendFactor, 0xFFFFFFFF);
            app.d3dContext->OMSetDepthStencilState(app.outlineDepthState.Get(), 0);
            app.d3dContext->RSSetState(app.outlineRasterizerState.Get());

            for (const GlowLayer& layer : glowLayers)
            {
                DrawCubePart(
                    viewProjection,
                    position,
                    XMFLOAT3(scale, scale, scale),
                    rotation,
                    app.settings.visualGlowColour,
                    layer.alpha,
                    true,
                    visibleThickness * layer.widthMultiplier,
                    previewWidth,
                    previewHeight);
            }
        }

        // The normally shaded character is rendered after every glow layer.  It
        // covers all interior outline fragments, leaving glow only outside the
        // exact silhouette while preserving normal lighting and depth testing.
        app.d3dContext->OMSetBlendState(
            app.alphaBlend.Get(), blendFactor, 0xFFFFFFFF);
        app.d3dContext->OMSetDepthStencilState(app.depthState.Get(), 0);
        app.d3dContext->RSSetState(app.rasterizerState.Get());
        DrawCubePart(
            viewProjection,
            position,
            XMFLOAT3(scale, scale, scale),
            rotation,
            app.settings.modelColour,
            1.0f,
            false,
            0.0f,
            previewWidth,
            previewHeight);
    }

