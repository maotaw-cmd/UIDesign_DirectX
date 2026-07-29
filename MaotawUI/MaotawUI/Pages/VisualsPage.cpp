#include "../Core/App.h"
#include "../CharacterPreviewMesh.h"

void App::DrawSidebar(float height)
{
    FillRect(0.0f, 0.0f, Layout::Sidebar, height, Theme::Sidebar);
    DrawLine(Layout::Sidebar - 0.5f, 0.0f, Layout::Sidebar - 0.5f, height, Theme::LineSoft);

    SidebarButton(0, Glyph::Target, 18.0f);

    const float saveTop = height - 108.0f;
    const bool saveHovered = Hover(14.0f, saveTop, 58.0f, saveTop + 43.0f);
    if (saveHovered || configPopupOpen_)
        FillRound(14.0f, saveTop, 58.0f, saveTop + 43.0f, 5.0f,
            configPopupOpen_ ? WithAlpha(settings_.accentColor, 0.12f) : Theme::RowHover);
    DrawIcon(Glyph::Save, 14.0f, saveTop, 58.0f, saveTop + 43.0f,
        configPopupOpen_ ? settings_.accentColor : saveHovered ? Theme::Text : Theme::Muted);

    if (!configPopupOpen_ && !colorPickerOpen_ && openCombo_ == 0 && saveHovered && clicked_)
    {
        configPopupOpen_ = true;
        configNameFocused_ = false;
        openCombo_ = 0;
        targetPopupOpen_ = false;
        visualSettingsPopup_ = 0;
        RefreshConfigs();
        clickConsumed_ = true;
    }

    SidebarButton(1, Glyph::Settings, height - 61.0f);
}

VisualProfile& App::ActiveVisualProfile()
{
    return selectedVisualTarget_ == 0 ? settings_.local : settings_.enemy;
}

void App::DrawVisualTargetSelector()
{
    const float left = Layout::RightX;
    const float right = Layout::RightRight;
    const float top = 16.0f;
    const float bottom = 54.0f;
    const bool hovered = Hover(left, top, right, bottom);

    FillRound(left, top, right, bottom, 6.0f,
        hovered || targetPopupOpen_ ? Theme::RowHover : Theme::Row);
    DrawRound(left, top, right, bottom, 6.0f,
        targetPopupOpen_ ? settings_.accentColor : Theme::Line, 1.0f);

    const wchar_t* icon = selectedVisualTarget_ == 0 ? Glyph::Local : Glyph::Enemy;
    const wchar_t* label = selectedVisualTarget_ == 0 ? L"Local player" : L"Enemy players";

    FillRound(left + 9.0f, top + 7.0f, left + 33.0f, bottom - 7.0f, 5.0f,
        WithAlpha(settings_.accentColor, 0.14f));
    DrawIcon(icon, left + 9.0f, top + 7.0f, left + 33.0f, bottom - 7.0f,
        settings_.accentColor);
    DrawText(label, left + 42.0f, top, right - 35.0f, bottom, Theme::Text, small_);
    DrawIcon(Glyph::Chevron, right - 30.0f, top, right - 7.0f, bottom,
        targetPopupOpen_ ? settings_.accentColor : Theme::Muted);

    if (!colorPickerOpen_ && openCombo_ == 0 && hovered && clicked_)
    {
        targetPopupOpen_ = !targetPopupOpen_;
        openCombo_ = 0;
        clickConsumed_ = true;
    }
}

void App::DrawVisualTargetPopup()
{
    if (!targetPopupOpen_ || colorPickerOpen_)
        return;

    const float right = Layout::LeftRight;
    const float left = right - 154.0f;
    const float top = 48.0f;
    const float bottom = top + 64.0f;

    FillRound(left, top, right, bottom, 5.0f, Theme::Popup);
    DrawRound(left, top, right, bottom, 5.0f, Theme::Line, 1.0f);

    struct TargetItem { const wchar_t* label; const wchar_t* icon; int id; };
    const TargetItem items[] = {
        { L"Local", Glyph::Local, 0 },
        { L"Enemy", Glyph::Enemy, 1 }
    };

    for (int i = 0; i < 2; ++i)
    {
        const float itemTop = top + 4.0f + i * 28.0f;
        const float itemBottom = itemTop + 26.0f;
        const bool hovered = Hover(left + 4.0f, itemTop, right - 4.0f, itemBottom);
        const bool selected = selectedVisualTarget_ == items[i].id;

        if (selected)
            FillRound(left + 4.0f, itemTop, right - 4.0f, itemBottom, 4.0f,
                WithAlpha(settings_.accentColor, 0.11f));
        else if (hovered)
            DrawRound(left + 4.0f, itemTop, right - 4.0f, itemBottom, 4.0f,
                WithAlpha(Theme::White, 0.09f), 1.0f);

        DrawIcon(items[i].icon, left + 10.0f, itemTop, left + 34.0f, itemBottom,
            selected ? settings_.accentColor : Theme::Muted);
        DrawText(items[i].label, left + 38.0f, itemTop, right - 26.0f, itemBottom,
            selected ? Theme::Text : Theme::SubText, tiny_);

        if (selected)
            DrawIcon(Glyph::Check, right - 25.0f, itemTop, right - 6.0f, itemBottom,
                settings_.accentColor);

        if (hovered && clicked_)
        {
            selectedVisualTarget_ = items[i].id;
            targetPopupOpen_ = false;
            visualSettingsPopup_ = 0;
            clickConsumed_ = true;
        }
    }
}

void App::DrawVisualOptionRow(int id, const std::wstring& label, bool& value, float y)
{
    VisualProfile& visual = ActiveVisualProfile();
    const float left = Layout::LeftX;
    const float right = Layout::LeftRight;
    const float height = 26.0f;
    const float gearLeft = right - 25.0f;
    const float toggleLeft = right - 55.0f;
    const float toggleRight = right - 31.0f;
    const bool rowHovered = Hover(left, y, right, y + height);
    const bool toggleHovered = Hover(toggleLeft, y, toggleRight, y + height);
    const bool gearHovered = Hover(gearLeft, y, right, y + height);
    const bool popupSelected = visualSettingsPopup_ == id;

    D2D1_COLOR_F* firstColor = nullptr;
    D2D1_COLOR_F* secondColor = nullptr;
    switch (id)
    {
    case 2: firstColor = &visual.boxColor; secondColor = &visual.boxHiddenColor; break;
    case 3: firstColor = &visual.fillColor; break;
    case 4: firstColor = &visual.nameColor; break;
    case 5: firstColor = &visual.healthHighColor; secondColor = &visual.healthLowColor; break;
    case 6: firstColor = &visual.armorColor; secondColor = &visual.armorBackColor; break;
    case 7: firstColor = &visual.skeletonColor; break;
    case 8: firstColor = &visual.weaponColor; break;
    case 9: firstColor = &visual.distanceColor; break;
    case 10: firstColor = &visual.snaplineColor; break;
    case 11: firstColor = &visual.glowColor; break;
    case 12: firstColor = &visual.chamsVisibleColor; secondColor = &visual.chamsHiddenColor; break;
    case 13: firstColor = &visual.damageTextColor; break;
    case 14: firstColor = &visual.arrowColor; break;
    }

    const float swatchW = 14.0f;
    const float swatchGap = 3.0f;
    const float secondRight = toggleLeft - 7.0f;
    const float secondLeft = secondRight - swatchW;
    const float firstRight = secondColor ? secondLeft - swatchGap : secondRight;
    const float firstLeft = firstRight - swatchW;
    const float textRight = firstColor ? firstLeft - 7.0f : toggleLeft - 8.0f;

    if (rowHovered || popupSelected)
        FillRound(left, y, right, y + height, 4.0f,
            popupSelected ? WithAlpha(settings_.accentColor, 0.08f) : WithAlpha(Theme::RowHover, 0.48f));

    DrawText(label, left + 8.0f, y, textRight, y + height,
        rowHovered ? Theme::Text : Theme::SubText, small_);

    auto drawSwatch = [&](D2D1_COLOR_F* color, float swatchLeft, float swatchRight)
        {
            if (!color) return;
            const float top = y + 8.0f;
            const float bottom = y + 18.0f;
            FillRound(swatchLeft, top, swatchRight, bottom, 2.0f, *color);
            DrawRound(swatchLeft, top, swatchRight, bottom, 2.0f, WithAlpha(Theme::White, 0.15f));
        };
    drawSwatch(firstColor, firstLeft, firstRight);
    if (secondColor) drawSwatch(secondColor, secondLeft, secondRight);

    const float centerX = (toggleLeft + toggleRight) * 0.5f;
    const float centerY = y + height * 0.5f;
    SetBrush(value ? settings_.accentColor : Theme::Muted);
    renderTarget_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), 5.0f, 5.0f), brush_, 1.1f);
    if (value)
    {
        SetBrush(settings_.accentColor);
        renderTarget_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), 2.2f, 2.2f), brush_);
    }

    DrawIcon(Glyph::Settings, gearLeft, y, right, y + height,
        popupSelected ? settings_.accentColor : gearHovered ? Theme::Text : Theme::Muted);

    if (!colorPickerOpen_ && clicked_)
    {
        const bool firstHovered = firstColor && Hover(firstLeft - 2.0f, y + 5.0f, firstRight + 2.0f, y + 21.0f);
        const bool secondHovered = secondColor && Hover(secondLeft - 2.0f, y + 5.0f, secondRight + 2.0f, y + 21.0f);
        if (firstHovered)
        {
            OpenColorPicker(*firstColor);
            clickConsumed_ = true;
        }
        else if (secondHovered)
        {
            OpenColorPicker(*secondColor);
            clickConsumed_ = true;
        }
        else if (gearHovered)
        {
            visualSettingsPopup_ = popupSelected ? 0 : id;
            if (visualSettingsPopup_ != 0)
            {
                visualPopupX_ = gearLeft - 10.0f;
                visualPopupY_ = y + height + 4.0f;
            }
            openCombo_ = 0;
            targetPopupOpen_ = false;
            clickConsumed_ = true;
        }
        else if (toggleHovered || Hover(left, y, textRight, y + height))
        {
            value = !value;
            clickConsumed_ = true;
        }
    }
}

void App::DrawVisualSettingsPopup()
{
    if (visualSettingsPopup_ == 0 || colorPickerOpen_)
        return;

    VisualProfile& visual = ActiveVisualProfile();
    float cardHeight = 128.0f;
    switch (visualSettingsPopup_)
    {
    case 2: cardHeight = 226.0f; break;
    case 3: cardHeight = 151.0f; break;
    case 4: cardHeight = 146.0f; break;
    case 5: cardHeight = 282.0f; break;
    case 6: cardHeight = 242.0f; break;
    case 7: cardHeight = 151.0f; break;
    case 9: cardHeight = 151.0f; break;
    case 10: cardHeight = 146.0f; break;
    case 11: cardHeight = 151.0f; break;
    case 12: cardHeight = 146.0f; break;
    case 14: cardHeight = 151.0f; break;
    }

    const float cardWidth = 270.0f;
    visualPopupX_ = Clamp(visualPopupX_, Layout::Sidebar + 5.0f, Layout::Width - cardWidth - 8.0f);
    visualPopupY_ = Clamp(visualPopupY_, 18.0f, Layout::Height - cardHeight - 8.0f);

    if (visualPopupDragging_ && mouseDown_)
    {
        visualPopupX_ = Clamp(static_cast<float>(mouse_.x) - visualPopupDragOffsetX_, Layout::Sidebar + 5.0f, Layout::Width - cardWidth - 8.0f);
        visualPopupY_ = Clamp(static_cast<float>(mouse_.y) - visualPopupDragOffsetY_, 18.0f, Layout::Height - cardHeight - 8.0f);
    }

    const float left = visualPopupX_;
    const float right = left + cardWidth;
    const float top = visualPopupY_;
    const float bottom = top + cardHeight;

    const bool pointerInsideCard = Hit(static_cast<float>(mouse_.x),
        static_cast<float>(mouse_.y), left, top, right, bottom);

    // Clicking outside closes the card and consumes that click. Clicking blank
    // space inside also belongs to the card and can never pass to the window.
    if (clicked_ && !pointerInsideCard && openCombo_ == 0)
    {
        visualSettingsPopup_ = 0;
        visualPopupDragging_ = false;
        activeSlider_ = ActiveSlider::None;
        clickConsumed_ = true;
        return;
    }
    if (clicked_ && pointerInsideCard)
        clickConsumed_ = true;

    // Fully opaque floating card. The empty right preview remains untouched when closed.
    FillRound(left, top, right, bottom, 6.0f, Theme::Popup);
    DrawRound(left, top, right, bottom, 6.0f, Theme::Line, 1.0f);

    const wchar_t* title = L"Visual settings";
    switch (visualSettingsPopup_)
    {
    case 2: title = L"Box settings"; break;
    case 3: title = L"Fill settings"; break;
    case 4: title = L"Name settings"; break;
    case 5: title = L"Health bar settings"; break;
    case 6: title = L"Armor bar settings"; break;
    case 7: title = L"Skeleton settings"; break;
    case 8: title = L"Weapon settings"; break;
    case 9: title = L"Distance settings"; break;
    case 10: title = L"Snapline settings"; break;
    case 11: title = L"Glow settings"; break;
    case 12: title = L"Chams settings"; break;
    case 13: title = L"Damage text settings"; break;
    case 14: title = L"Arrow settings"; break;
    }

    const float headerBottom = top + 29.0f;
    const float closeLeft = right - 28.0f;
    const bool closeHovered = Hover(closeLeft, top, right - 4.0f, headerBottom);
    const bool dragHovered = Hover(left, top, closeLeft, headerBottom);

    // Header remains the same solid colour on hover.
    DrawText(title, left + 11.0f, top, closeLeft - 3.0f, headerBottom, Theme::Text, tiny_);
    DrawIcon(Glyph::Close, closeLeft, top, right - 4.0f, headerBottom,
        closeHovered ? Theme::Text : Theme::Muted);
    Divider(left + 9.0f, right - 9.0f, headerBottom + 1.0f);

    if (closeHovered && clicked_)
    {
        visualSettingsPopup_ = 0;
        openCombo_ = 0;
        visualPopupDragging_ = false;
        clickConsumed_ = true;
        return;
    }
    if (dragHovered && clicked_)
    {
        visualPopupDragging_ = true;
        visualPopupDragOffsetX_ = static_cast<float>(mouse_.x) - left;
        visualPopupDragOffsetY_ = static_cast<float>(mouse_.y) - top;
        clickConsumed_ = true;
    }

    const float x1 = left + 13.0f;
    const float x2 = right - 13.0f;
    const float y = top + 36.0f;

    switch (visualSettingsPopup_)
    {
    case 1:
        ToggleRow(L"Only when visible", visual.enabled, x1, x2, y);
        break;
    case 2:
        ComboControl(210, L"Box style", { L"Cornered", L"Full box", L"3D box" }, visual.boxStyle, x1, x2, y);
        SliderControl(L"Thickness", visual.boxThickness, 0.5f, 4.0f, x1, x2, y + 53.0f, ActiveSlider::BoxThickness, 1);
        ColorRow(L"Visible / main", visual.boxColor, x1, x2, y + 106.0f);
        ColorRow(L"Hidden", visual.boxHiddenColor, x1, x2, y + 146.0f);
        break;
    case 3:
        SliderControl(L"Fill opacity", visual.fillOpacity, 0.0f, 100.0f, x1, x2, y, ActiveSlider::FillOpacity, 0);
        ColorRow(L"Fill color", visual.fillColor, x1, x2, y + 53.0f);
        break;
    case 4:
        ComboControl(211, L"Name position", { L"Top", L"Bottom" }, visual.namePosition, x1, x2, y);
        ColorRow(L"Text color", visual.nameColor, x1, x2, y + 53.0f);
        break;
    case 5:
        ComboControl(212, L"Bar position", { L"Left", L"Right", L"Top", L"Bottom" }, visual.healthPosition, x1, x2, y);
        ComboControl(215, L"Bar style", { L"Normal", L"Segmented", L"Gradient" }, visual.healthStyle, x1, x2, y + 53.0f);
        ColorRow(L"Healthy color", visual.healthHighColor, x1, x2, y + 106.0f);
        ColorRow(L"Low health color", visual.healthLowColor, x1, x2, y + 146.0f);
        ColorRow(L"Background", visual.healthBackColor, x1, x2, y + 186.0f);
        break;
    case 6:
        ComboControl(214, L"Bar position", { L"Left", L"Right", L"Top", L"Bottom" }, visual.armorPosition, x1, x2, y);
        ComboControl(216, L"Bar style", { L"Normal", L"Segmented", L"Gradient" }, visual.armorStyle, x1, x2, y + 53.0f);
        ColorRow(L"Armor color", visual.armorColor, x1, x2, y + 106.0f);
        ColorRow(L"Background", visual.armorBackColor, x1, x2, y + 146.0f);
        break;
    case 7:
        SliderControl(L"Thickness", visual.skeletonThickness, 0.5f, 4.0f, x1, x2, y, ActiveSlider::SkeletonThickness, 1);
        ColorRow(L"Skeleton color", visual.skeletonColor, x1, x2, y + 53.0f);
        break;
    case 8:
        ColorRow(L"Weapon text color", visual.weaponColor, x1, x2, y);
        break;
    case 9:
        SliderControl(L"Maximum distance", visual.maxDistance, 25.0f, 500.0f, x1, x2, y, ActiveSlider::MaxDistance, 0);
        ColorRow(L"Distance color", visual.distanceColor, x1, x2, y + 53.0f);
        break;
    case 10:
        ComboControl(213, L"Line origin", { L"Bottom", L"Centre", L"Top" }, visual.snaplinePosition, x1, x2, y);
        ColorRow(L"Line color", visual.snaplineColor, x1, x2, y + 53.0f);
        break;
    case 11:
        SliderControl(L"Glow strength", visual.glowStrength, 0.0f, 100.0f, x1, x2, y, ActiveSlider::GlowStrength, 0);
        ColorRow(L"Glow color", visual.glowColor, x1, x2, y + 53.0f);
        break;
    case 12:
        ColorRow(L"Visible color", visual.chamsVisibleColor, x1, x2, y);
        ColorRow(L"Hidden color", visual.chamsHiddenColor, x1, x2, y + 40.0f);
        break;
    case 13:
        ColorRow(L"Damage text color", visual.damageTextColor, x1, x2, y);
        break;
    case 14:
        SliderControl(L"Arrow size", visual.arrowSize, 6.0f, 30.0f, x1, x2, y, ActiveSlider::ArrowSize, 0);
        ColorRow(L"Arrow color", visual.arrowColor, x1, x2, y + 53.0f);
        break;
    }
}

void App::Draw3DCharacterPreview()
{
    VisualProfile& visual = ActiveVisualProfile();

    const float panelLeft = Layout::RightX;
    const float panelTop = 68.0f;
    const float panelRight = Layout::RightRight;
    const float panelBottom = 438.0f;

    DrawText(L"LIVE PREVIEW", panelLeft + 12.0f, panelTop + 7.0f,
        panelRight - 12.0f, panelTop + 27.0f, Theme::Muted, tiny_);

    const bool drawEsp = visual.enabled;

    const float viewLeft = panelLeft + 24.0f;
    const float viewTop = panelTop + 34.0f;
    const float viewRight = panelRight - 24.0f;
    const float viewBottom = panelBottom - 24.0f;
    const float viewHeight = viewBottom - viewTop;

    struct ProjectedVertex
    {
        float x, y, z;
        float light;
        float facing;
        D2D1_COLOR_F color;
    };
    struct Triangle
    {
        ProjectedVertex a, b, c;
        float depth;
    };

    const float time = static_cast<float>(GetTickCount64() % 100000ULL) / 1000.0f;
    const float yaw = time * 0.34f;
    const float cs = std::cos(yaw);
    const float sn = std::sin(yaw);
    const float scale = viewHeight * 0.465f;
    const float centerX = (viewLeft + viewRight) * 0.5f;
    const float feetY = viewBottom - 12.0f;
    const float cameraDistance = 3.65f;

    auto project = [&](const PreviewMeshVertex& vertex) -> ProjectedVertex
    {
        const float px = vertex.position.x;
        const float py = vertex.position.y;
        const float pz = vertex.position.z;
        const float rx = px * cs + pz * sn;
        const float rz = -px * sn + pz * cs;

        const float nx = vertex.normal.x * cs + vertex.normal.z * sn;
        const float nz = -vertex.normal.x * sn + vertex.normal.z * cs;
        const float light = Clamp(0.25f + 0.58f * (-nz) + 0.22f * vertex.normal.y, 0.18f, 1.0f);
        const float perspective = cameraDistance / std::max(1.0f, cameraDistance + rz);

        return {
            centerX + rx * scale * perspective,
            feetY - py * scale * perspective,
            rz,
            light,
            -nz,
            D2D1::ColorF(vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a)
        };
    };

    // Reuse the allocation between frames; the baked model contains many
    // triangles and repeatedly allocating this vector caused avoidable stalls.
    static std::vector<Triangle> triangles;
    triangles.clear();
    if (triangles.capacity() < kPreviewIndexCount / 3)
        triangles.reserve(kPreviewIndexCount / 3);
    for (UINT index = 0; index + 2 < kPreviewIndexCount; index += 3)
    {
        const ProjectedVertex a = project(kPreviewVertices[kPreviewIndices[index]]);
        const ProjectedVertex b = project(kPreviewVertices[kPreviewIndices[index + 1]]);
        const ProjectedVertex c = project(kPreviewVertices[kPreviewIndices[index + 2]]);
        const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (std::abs(cross) < 0.0001f)
            continue;

        // Cull with the imported vertex normals instead of triangle winding.
        // The GLB has mixed winding in some submeshes, but its normals are
        // consistent. This safely removes most hidden triangles and nearly
        // halves Direct2D geometry creation without making the model vanish.
        const float averageFacing = (a.facing + b.facing + c.facing) / 3.0f;
        if (averageFacing < -0.12f)
            continue;

        triangles.push_back({ a, b, c, (a.z + b.z + c.z) / 3.0f });
    }

    std::sort(triangles.begin(), triangles.end(),
        [](const Triangle& left, const Triangle& right) { return left.depth > right.depth; });

    const float boxLeft = centerX - 66.0f;
    const float boxTop = viewTop + 10.0f;
    const float boxRight = centerX + 66.0f;
    const float boxBottom = viewBottom - 5.0f;

    // Filled ESP belongs behind the model. Drawing it after the model made the
    // character appear to disappear, especially at higher opacity values.
    if (drawEsp && visual.filledBox)
        FillRect(boxLeft, boxTop, boxRight, boxBottom,
            WithAlpha(visual.fillColor, Clamp(visual.fillOpacity / 100.0f, 0.0f, 0.75f)));

    // True model silhouette glow. Build one geometry from the actual projected
    // mesh, draw several soft outline widths, then fill the textured model over
    // it. The model masks internal triangle edges, leaving the outside silhouette.
    if (drawEsp && visual.glow && visual.glowStrength > 0.01f)
    {
        ID2D1PathGeometry* outlineGeometry = nullptr;
        ID2D1GeometrySink* outlineSink = nullptr;

        if (SUCCEEDED(d2dFactory_->CreatePathGeometry(&outlineGeometry)) &&
            SUCCEEDED(outlineGeometry->Open(&outlineSink)))
        {
            for (const Triangle& triangle : triangles)
            {
                outlineSink->BeginFigure(
                    D2D1::Point2F(triangle.a.x, triangle.a.y),
                    D2D1_FIGURE_BEGIN_HOLLOW);
                outlineSink->AddLine(D2D1::Point2F(triangle.b.x, triangle.b.y));
                outlineSink->AddLine(D2D1::Point2F(triangle.c.x, triangle.c.y));
                outlineSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            }

            if (SUCCEEDED(outlineSink->Close()))
            {
                struct GlowLayer
                {
                    float widthMultiplier;
                    float alpha;
                };

                static constexpr GlowLayer glowLayers[] =
                {
                    { 2.60f, 0.09f },
                    { 1.55f, 0.24f },
                    { 1.00f, 0.86f }
                };

                const float strength = Clamp(visual.glowStrength / 100.0f, 0.0f, 1.0f);
                const float baseStroke = 0.85f + strength * 5.15f;

                for (const GlowLayer& layer : glowLayers)
                {
                    SetBrush(WithAlpha(visual.glowColor, layer.alpha));
                    renderTarget_->DrawGeometry(
                        outlineGeometry,
                        brush_,
                        baseStroke * layer.widthMultiplier);
                }
            }
        }

        SafeRelease(outlineSink);
        SafeRelease(outlineGeometry);
    }

    // Textured/chams model pass drawn after glow to mask its interior.
    for (const Triangle& triangle : triangles)
    {
        ID2D1PathGeometry* geometry = nullptr;
        ID2D1GeometrySink* sink = nullptr;
        if (FAILED(d2dFactory_->CreatePathGeometry(&geometry)) || FAILED(geometry->Open(&sink)))
        {
            SafeRelease(sink);
            SafeRelease(geometry);
            continue;
        }
        sink->BeginFigure(D2D1::Point2F(triangle.a.x, triangle.a.y), D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(triangle.b.x, triangle.b.y));
        sink->AddLine(D2D1::Point2F(triangle.c.x, triangle.c.y));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();

        const float light = (triangle.a.light + triangle.b.light + triangle.c.light) / 3.0f;
        D2D1_COLOR_F modelColor = (drawEsp && visual.chams)
            ? visual.chamsVisibleColor
            : D2D1::ColorF(
                (triangle.a.color.r + triangle.b.color.r + triangle.c.color.r) / 3.0f,
                (triangle.a.color.g + triangle.b.color.g + triangle.c.color.g) / 3.0f,
                (triangle.a.color.b + triangle.b.color.b + triangle.c.color.b) / 3.0f,
                (triangle.a.color.a + triangle.b.color.a + triangle.c.color.a) / 3.0f);
        SetBrush(D2D1::ColorF(
            Clamp(modelColor.r * light, 0.0f, 1.0f),
            Clamp(modelColor.g * light, 0.0f, 1.0f),
            Clamp(modelColor.b * light, 0.0f, 1.0f), modelColor.a));
        renderTarget_->FillGeometry(geometry, brush_);
        SafeRelease(sink);
        SafeRelease(geometry);
    }

    if (drawEsp && visual.box)
    {
        const float stroke = Clamp(visual.boxThickness, 0.5f, 4.0f);
        const float corner = 22.0f;
        visual.boxStyle = std::clamp(visual.boxStyle, 0, 2);
        if (visual.boxStyle == 0) // Cornered
        {
            DrawLine(boxLeft, boxTop, boxLeft + corner, boxTop, visual.boxColor, stroke);
            DrawLine(boxLeft, boxTop, boxLeft, boxTop + corner, visual.boxColor, stroke);
            DrawLine(boxRight - corner, boxTop, boxRight, boxTop, visual.boxColor, stroke);
            DrawLine(boxRight, boxTop, boxRight, boxTop + corner, visual.boxColor, stroke);
            DrawLine(boxLeft, boxBottom, boxLeft + corner, boxBottom, visual.boxColor, stroke);
            DrawLine(boxLeft, boxBottom - corner, boxLeft, boxBottom, visual.boxColor, stroke);
            DrawLine(boxRight - corner, boxBottom, boxRight, boxBottom, visual.boxColor, stroke);
            DrawLine(boxRight, boxBottom - corner, boxRight, boxBottom, visual.boxColor, stroke);
        }
        else if (visual.boxStyle == 1) // Full box
        {
            DrawRect(boxLeft, boxTop, boxRight, boxBottom, visual.boxColor, stroke);
        }
        else // 3D box
        {
            const float offsetX = 10.0f;
            const float offsetY = -8.0f;
            DrawRect(boxLeft, boxTop, boxRight, boxBottom, visual.boxColor, stroke);
            DrawRect(boxLeft + offsetX, boxTop + offsetY, boxRight + offsetX, boxBottom + offsetY, visual.boxColor, stroke);
            DrawLine(boxLeft, boxTop, boxLeft + offsetX, boxTop + offsetY, visual.boxColor, stroke);
            DrawLine(boxRight, boxTop, boxRight + offsetX, boxTop + offsetY, visual.boxColor, stroke);
            DrawLine(boxLeft, boxBottom, boxLeft + offsetX, boxBottom + offsetY, visual.boxColor, stroke);
            DrawLine(boxRight, boxBottom, boxRight + offsetX, boxBottom + offsetY, visual.boxColor, stroke);
        }
    }

    auto MixColor = [](const D2D1_COLOR_F& a, const D2D1_COLOR_F& b, float t)
    {
        t = Clamp(t, 0.0f, 1.0f);
        return D2D1::ColorF(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t);
    };

    auto DrawPreviewBar = [&](float value, int position, int style,
        const D2D1_COLOR_F& lowColor, const D2D1_COLOR_F& highColor,
        const D2D1_COLOR_F& background, float sideOffset, float thickness)
    {
        value = Clamp(value, 0.0f, 1.0f);
        style = std::clamp(style, 0, 2);
        position = std::clamp(position, 0, 3);
        const bool horizontal = position >= 2;
        float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;

        if (horizontal)
        {
            left = boxLeft;
            right = boxRight;
            if (position == 2) // Top
            {
                top = boxTop - sideOffset;
                bottom = top + thickness;
            }
            else // Bottom
            {
                top = boxBottom + sideOffset - thickness;
                bottom = top + thickness;
            }
        }
        else
        {
            left = position == 0 ? boxLeft - sideOffset : boxRight + sideOffset - thickness;
            right = left + thickness;
            top = boxTop;
            bottom = boxBottom;
        }

        FillRound(left, top, right, bottom, 1.5f, background);

        if (style == 0) // Normal
        {
            const D2D1_COLOR_F color = MixColor(lowColor, highColor, value);
            if (horizontal)
                FillRound(left, top, left + (right - left) * value, bottom, 1.5f, color);
            else
                FillRound(left, bottom - (bottom - top) * value, right, bottom, 1.5f, color);
        }
        else if (style == 1) // Segmented
        {
            constexpr int segments = 10;
            const float gap = 1.0f;
            const int active = static_cast<int>(std::ceil(value * segments));
            for (int i = 0; i < segments; ++i)
            {
                const float segmentValue = static_cast<float>(i + 1) / static_cast<float>(segments);
                const D2D1_COLOR_F color = i < active ? MixColor(lowColor, highColor, segmentValue) : background;
                if (horizontal)
                {
                    const float segmentWidth = (right - left - gap * (segments - 1)) / segments;
                    const float x1 = left + i * (segmentWidth + gap);
                    FillRect(x1, top, x1 + segmentWidth, bottom, color);
                }
                else
                {
                    const float segmentHeight = (bottom - top - gap * (segments - 1)) / segments;
                    const float y2 = bottom - i * (segmentHeight + gap);
                    FillRect(left, y2 - segmentHeight, right, y2, color);
                }
            }
        }
        else // Gradient
        {
            constexpr int steps = 48;
            const int visibleSteps = std::max(1, static_cast<int>(std::ceil(value * steps)));
            for (int i = 0; i < visibleSteps; ++i)
            {
                const float t0 = static_cast<float>(i) / static_cast<float>(steps);
                const float t1 = static_cast<float>(i + 1) / static_cast<float>(steps);
                const D2D1_COLOR_F color = MixColor(lowColor, highColor, t1);
                if (horizontal)
                    FillRect(left + (right - left) * t0, top,
                        left + (right - left) * std::min(t1, value), bottom, color);
                else
                    FillRect(left, bottom - (bottom - top) * std::min(t1, value),
                        right, bottom - (bottom - top) * t0, color);
            }
        }
    };

    const float health = 0.78f;
    if (drawEsp && visual.healthBar)
        DrawPreviewBar(health, visual.healthPosition, visual.healthStyle,
            visual.healthLowColor, visual.healthHighColor, visual.healthBackColor, 10.0f, 5.0f);

    const float armor = 0.62f;
    if (drawEsp && visual.armorBar)
        DrawPreviewBar(armor, visual.armorPosition, visual.armorStyle,
            visual.armorColor, visual.armorColor, visual.armorBackColor, 17.0f, 4.0f);

    if (drawEsp && visual.name)
    {
        const float top = visual.namePosition == 0 ? boxTop - 24.0f : boxBottom + 2.0f;
        DrawText(L"ENEMY", boxLeft, top, boxRight, top + 20.0f,
            visual.nameColor, tiny_, DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    // Keep weapon and distance clear of bottom-positioned bars. The outermost
    // enabled bar decides how far the text is pushed down.
    float bottomBarClearance = 0.0f;
    if (visual.healthBar && visual.healthPosition == 3)
        bottomBarClearance = std::max(bottomBarClearance, 12.0f);
    if (visual.armorBar && visual.armorPosition == 3)
        bottomBarClearance = std::max(bottomBarClearance, 19.0f);

    float bottomTextY = boxBottom + bottomBarClearance +
        (visual.name && visual.namePosition == 1 ? 18.0f : 2.0f);
    if (drawEsp && visual.weapon)
    {
        DrawText(L"AK-47", boxLeft, bottomTextY, boxRight, bottomTextY + 18.0f,
            visual.weaponColor, tiny_, DWRITE_TEXT_ALIGNMENT_CENTER);
        bottomTextY += 15.0f;
    }
    if (drawEsp && visual.distance)
        DrawText(L"42 m", boxLeft, bottomTextY, boxRight, bottomTextY + 18.0f,
            visual.distanceColor, tiny_, DWRITE_TEXT_ALIGNMENT_CENTER);

    if (drawEsp && visual.snaplines)
    {
        const float startY = visual.snaplinePosition == 0 ? viewBottom :
            visual.snaplinePosition == 1 ? (viewTop + viewBottom) * 0.5f : viewTop;
        DrawLine((viewLeft + viewRight) * 0.5f, startY,
            (boxLeft + boxRight) * 0.5f, boxBottom, visual.snaplineColor, 1.0f);
    }

    if (drawEsp && visual.damageText)
    {
        // Preview a damage hit: pop near the upper torso, rise, drift and fade.
        // The cycle repeats so the animation is always visible in the editor.
        constexpr float cycleMs = 1450.0f;
        const float elapsed = static_cast<float>(GetTickCount64() % static_cast<ULONGLONG>(cycleMs));
        const float progress = Clamp(elapsed / cycleMs, 0.0f, 1.0f);
        const float eased = 1.0f - (1.0f - progress) * (1.0f - progress);
        const float fade = Clamp(1.0f - progress * 1.18f, 0.0f, 1.0f);
        const float pop = std::sin(std::min(progress * 3.4f, 1.0f) * 1.5707963f);

        const float textWidth = 54.0f + pop * 5.0f;
        const float textHeight = 25.0f + pop * 3.0f;
        const float centerX = boxRight + 13.0f + std::sin(progress * 5.2f) * 4.5f;
        const float centerY = boxTop + 104.0f - eased * 50.0f;
        const float left = centerX - textWidth * 0.5f;
        const float top = centerY - textHeight * 0.5f;

        // A small dark shadow keeps white damage numbers readable on textures.
        DrawText(L"-24", left + 1.5f, top + 1.5f,
            left + textWidth + 1.5f, top + textHeight + 1.5f,
            MakeColor(0x000000, fade * 0.72f), damageText_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
        DrawText(L"-24", left, top, left + textWidth, top + textHeight,
            WithAlpha(visual.damageTextColor, fade), damageText_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    if (!drawEsp)
        DrawText(L"Visuals disabled", panelLeft + 20.0f, panelBottom - 42.0f,
            panelRight - 20.0f, panelBottom - 20.0f, Theme::Muted, tiny_,
            DWRITE_TEXT_ALIGNMENT_CENTER);
}

void App::DrawMainPage()
{
    DrawLine(Layout::SplitX + 0.5f, 13.0f, Layout::SplitX + 0.5f, 447.0f, Theme::LineSoft);

    DrawText(L"Visuals", Layout::LeftX, 16.0f, Layout::LeftRight - 42.0f, 44.0f,
        Theme::Text, text_);

    // Compact profile icon in the top-right of the Visuals column.
    const float profileLeft = Layout::LeftRight - 34.0f;
    const float profileRight = Layout::LeftRight;
    const float profileTop = 14.0f;
    const float profileBottom = 46.0f;
    const bool profileHovered = Hover(profileLeft, profileTop, profileRight, profileBottom);
    const wchar_t* profileIcon = selectedVisualTarget_ == 0 ? Glyph::Local : Glyph::Enemy;

    if (targetPopupOpen_)
        FillRound(profileLeft, profileTop, profileRight, profileBottom, 5.0f,
            WithAlpha(settings_.accentColor, 0.11f));
    else if (profileHovered)
        DrawRound(profileLeft, profileTop, profileRight, profileBottom, 5.0f,
            WithAlpha(Theme::White, 0.11f), 1.0f);

    DrawIcon(profileIcon, profileLeft, profileTop, profileRight, profileBottom,
        targetPopupOpen_ ? settings_.accentColor : profileHovered ? Theme::Text : Theme::Muted);

    if (!colorPickerOpen_ && profileHovered && clicked_)
    {
        targetPopupOpen_ = !targetPopupOpen_;
        visualSettingsPopup_ = 0;
        openCombo_ = 0;
        clickConsumed_ = true;
    }

    VisualProfile& visual = ActiveVisualProfile();
    DrawText(selectedVisualTarget_ == 0 ? L"LOCAL PROFILE" : L"ENEMY PROFILE",
        Layout::LeftX, 45.0f, Layout::LeftRight, 62.0f, Theme::Muted, tiny_);

    const float firstY = 64.0f;
    const float step = 27.0f;
    DrawVisualOptionRow(1, L"Enable visuals", visual.enabled, firstY + step * 0.0f);
    DrawVisualOptionRow(2, L"Box", visual.box, firstY + step * 1.0f);
    DrawVisualOptionRow(3, L"Filled box", visual.filledBox, firstY + step * 2.0f);
    DrawVisualOptionRow(4, L"Name", visual.name, firstY + step * 3.0f);
    DrawVisualOptionRow(5, L"Health bar", visual.healthBar, firstY + step * 4.0f);
    DrawVisualOptionRow(6, L"Armor bar", visual.armorBar, firstY + step * 5.0f);
    DrawVisualOptionRow(7, L"Skeleton", visual.skeleton, firstY + step * 6.0f);
    DrawVisualOptionRow(8, L"Weapon", visual.weapon, firstY + step * 7.0f);
    DrawVisualOptionRow(9, L"Distance", visual.distance, firstY + step * 8.0f);
    DrawVisualOptionRow(10, L"Snaplines", visual.snaplines, firstY + step * 9.0f);
    DrawVisualOptionRow(11, L"Glow", visual.glow, firstY + step * 10.0f);
    DrawVisualOptionRow(12, L"Chams", visual.chams, firstY + step * 11.0f);
    DrawVisualOptionRow(13, L"Damage text", visual.damageText, firstY + step * 12.0f);
    DrawVisualOptionRow(14, L"Off-screen arrows", visual.offscreenArrows, firstY + step * 13.0f);

    Draw3DCharacterPreview();
}

void App::DrawShell(float width, float height)
{
    const BYTE windowAlpha = static_cast<BYTE>(std::clamp(
        static_cast<int>(std::round(settings_.windowOpacity * 2.55f)), 64, 255));
    SetLayeredWindowAttributes(hwnd_, 0, windowAlpha, LWA_ALPHA);

    FillRect(0.0f, 0.0f, width, height, Theme::Window);
    DrawConnectedParticles(width, height);

    // Keep sidebar navigation live even if a stale selector/card state exists.
    // Only the page content behind a floating card is input-disabled.
    const bool savedClicked = clicked_;
    const POINT savedMouse = mouse_;
    const bool modalCardOpen = configPopupOpen_ || visualSettingsPopup_ != 0 || targetPopupOpen_;

    DrawSidebar(height);

    if (modalCardOpen)
    {
        clicked_ = false;
        mouse_ = POINT{ -10000, -10000 };
    }

    // Route built-in pages directly. This prevents Settings from being lost
    // when custom-page routing or popup state changes.
    switch (selectedPage_)
    {
    case 0:
        DrawMainPage();
        break;
    case 1:
        DrawSettingsPage();
        break;
    case 3:
        DrawProfilePage();
        break;
    default:
        DrawOtherPage(selectedPage_);
        break;
    }

    if (modalCardOpen)
    {
        mouse_ = savedMouse;
        clicked_ = savedClicked;
    }

    // Normal floating UI is available only when no modal card is open.
    if (!configPopupOpen_ && visualSettingsPopup_ == 0)
    {
        DrawComboPopup();
        DrawVisualTargetPopup();
        DrawColorPicker();
    }

    // Visual settings owns all input while open. Its dropdown and colour picker
    // are rendered afterwards so they remain interactive above the card.
    if (!configPopupOpen_ && visualSettingsPopup_ != 0)
    {
        DrawVisualSettingsPopup();
        DrawComboPopup();
        DrawColorPicker();
    }

    // Config popup has the highest modal priority.
    if (configPopupOpen_)
        DrawConfigPopup();

    // Draw the outline after every page and the sidebar so no panel can cover it.
    if (settings_.windowBorder && settings_.windowBorderOpacity > 0.0f)
    {
        const float outlineAlpha = Clamp(settings_.windowBorderOpacity / 100.0f, 0.0f, 1.0f);
        DrawRound(1.0f, 1.0f, width - 1.0f, height - 1.0f, 10.0f,
            WithAlpha(Theme::White, 0.22f * outlineAlpha), 2.0f);
        DrawRound(2.0f, 2.0f, width - 2.0f, height - 2.0f, 9.0f,
            WithAlpha(settings_.accentColor, outlineAlpha), 1.0f);
    }

    if (!configPopupOpen_ && visualSettingsPopup_ == 0 &&
        !colorPickerOpen_ && clicked_ && !clickConsumed_)
    {
        openCombo_ = 0;
        targetPopupOpen_ = false;
        configNameFocused_ = false;
    }
}

