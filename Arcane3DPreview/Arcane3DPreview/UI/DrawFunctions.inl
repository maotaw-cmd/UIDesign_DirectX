// Included inside class App.
// All shared drawing primitives, widgets, icons and popup rendering live here.

    D2D1_COLOR_F C(unsigned rgb, float a=1.f) {
        return D2D1::ColorF(((rgb>>16)&255)/255.f, ((rgb>>8)&255)/255.f, (rgb&255)/255.f, a);
    }
    D2D1_COLOR_F C(const RGB& c, float a=1.f) {
        return D2D1::ColorF(c.r, c.g, c.b, a);
    }
    void Fill(D2D1_RECT_F r, unsigned c, float a=1.f) { brush->SetColor(C(c,a)); rt->FillRectangle(r, brush.Get()); }
    void Fill(D2D1_RECT_F r, const RGB& c, float a=1.f) { brush->SetColor(C(c,a)); rt->FillRectangle(r, brush.Get()); }
    void Stroke(D2D1_RECT_F r, unsigned c, float w=1.f) { brush->SetColor(C(c)); rt->DrawRectangle(r, brush.Get(), w); }
    void Round(D2D1_RECT_F r, float rad, unsigned c, float a=1.f) { brush->SetColor(C(c,a)); rt->FillRoundedRectangle(D2D1::RoundedRect(r,rad,rad), brush.Get()); }
    void Round(D2D1_RECT_F r, float rad, const RGB& c, float a=1.f) { brush->SetColor(C(c,a)); rt->FillRoundedRectangle(D2D1::RoundedRect(r,rad,rad), brush.Get()); }
    void RoundStroke(D2D1_RECT_F r, float rad, unsigned c, float w=1.f) { brush->SetColor(C(c)); rt->DrawRoundedRectangle(D2D1::RoundedRect(r,rad,rad), brush.Get(), w); }
    void Line(float x1, float y1, float x2, float y2, unsigned c, float w=1.f) { brush->SetColor(C(c)); rt->DrawLine({x1,y1},{x2,y2}, brush.Get(), w); }
    void Circle(float x, float y, float r, unsigned c) { brush->SetColor(C(c)); rt->FillEllipse(D2D1::Ellipse({x,y},r,r), brush.Get()); }
    void Text(const std::wstring& s, float x, float y, float w, float h, unsigned c,
              IDWriteTextFormat* fmt=nullptr,
              DWRITE_TEXT_ALIGNMENT align=DWRITE_TEXT_ALIGNMENT_LEADING,
              DWRITE_PARAGRAPH_ALIGNMENT palign=DWRITE_PARAGRAPH_ALIGNMENT_CENTER) {
        if (!fmt) fmt = f10.Get();
        fmt->SetTextAlignment(align);
        fmt->SetParagraphAlignment(palign);
        brush->SetColor(C(c));
        rt->DrawTextW(s.c_str(), (UINT32)s.size(), fmt, D2D1::RectF(x,y,x+w,y+h), brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }
    void TriangleLogo(float x, float y) {
        D2D1_POINT_2F pts[3] = {{x+10,y},{x+20,y+19},{x,y+19}};
        ComPtr<ID2D1PathGeometry> geo; factory->CreatePathGeometry(&geo);
        ComPtr<ID2D1GeometrySink> sink; geo->Open(&sink);
        sink->BeginFigure(pts[0], D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(pts[1]); sink->AddLine(pts[2]);
        sink->EndFigure(D2D1_FIGURE_END_CLOSED); sink->Close();
        brush->SetColor(C(0xE8EAEB)); rt->FillGeometry(geo.Get(), brush.Get());

        D2D1_POINT_2F inner[3] = {{x+10,y+6},{x+15,y+16},{x+5,y+16}};
        ComPtr<ID2D1PathGeometry> geo2; factory->CreatePathGeometry(&geo2);
        ComPtr<ID2D1GeometrySink> sink2; geo2->Open(&sink2);
        sink2->BeginFigure(inner[0], D2D1_FIGURE_BEGIN_FILLED);
        sink2->AddLine(inner[1]); sink2->AddLine(inner[2]);
        sink2->EndFigure(D2D1_FIGURE_END_CLOSED); sink2->Close();
        brush->SetColor(C(0x121517)); rt->FillGeometry(geo2.Get(), brush.Get());
    }
    void DrawAimbotIcon(float cx, float cy, unsigned col) {
        // Minimal Fluent-style focus/target icon.
        const float o = 7.5f, i = 3.8f;
        Line(cx-o,cy-i,cx-o,cy-o,col,1.35f); Line(cx-o,cy-o,cx-i,cy-o,col,1.35f);
        Line(cx+i,cy-o,cx+o,cy-o,col,1.35f); Line(cx+o,cy-o,cx+o,cy-i,col,1.35f);
        Line(cx-o,cy+i,cx-o,cy+o,col,1.35f); Line(cx-o,cy+o,cx-i,cy+o,col,1.35f);
        Line(cx+i,cy+o,cx+o,cy+o,col,1.35f); Line(cx+o,cy+i,cx+o,cy+o,col,1.35f);
        brush->SetColor(C(col));
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},2.5f,2.5f), brush.Get(), 1.25f);
        Circle(cx,cy,0.9f,col);
    }
    void DrawVisualsIcon(float cx, float cy, unsigned col) {
        // Clean Fluent-style eye.
        ComPtr<ID2D1PathGeometry> geo; factory->CreatePathGeometry(&geo);
        ComPtr<ID2D1GeometrySink> sink; geo->Open(&sink);
        sink->BeginFigure({cx-8.5f,cy}, D2D1_FIGURE_BEGIN_HOLLOW);
        sink->AddBezier(D2D1::BezierSegment({cx-4.8f,cy-5.1f},{cx+4.8f,cy-5.1f},{cx+8.5f,cy}));
        sink->AddBezier(D2D1::BezierSegment({cx+4.8f,cy+5.1f},{cx-4.8f,cy+5.1f},{cx-8.5f,cy}));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED); sink->Close();
        brush->SetColor(C(col)); rt->DrawGeometry(geo.Get(),brush.Get(),1.3f);
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},2.4f,2.4f), brush.Get(), 1.3f);
        Circle(cx,cy,0.85f,col);
    }
    void DrawMiscIcon(float cx, float cy, unsigned col) {
        // Compact tune/sliders icon with outlined knobs.
        brush->SetColor(C(col));
        Line(cx-8,cy-5.5f,cx+8,cy-5.5f,col,1.25f);
        Line(cx-8,cy,cx+8,cy,col,1.25f);
        Line(cx-8,cy+5.5f,cx+8,cy+5.5f,col,1.25f);
        rt->DrawEllipse(D2D1::Ellipse({cx-3.5f,cy-5.5f},2.0f,2.0f),brush.Get(),1.25f);
        rt->DrawEllipse(D2D1::Ellipse({cx+4.0f,cy},2.0f,2.0f),brush.Get(),1.25f);
        rt->DrawEllipse(D2D1::Ellipse({cx-0.5f,cy+5.5f},2.0f,2.0f),brush.Get(),1.25f);
    }
    void DrawSettingsIcon(float cx, float cy, unsigned col) {
        // Rounded six-tooth gear, less busy than the previous radial lines.
        brush->SetColor(C(col));
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},5.2f,5.2f), brush.Get(), 1.35f);
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},1.7f,1.7f), brush.Get(), 1.2f);
        for (int i=0;i<6;i++) {
            const float a=6.2831853f*i/6.0f;
            const float dx=std::cos(a), dy=std::sin(a);
            Line(cx+6.1f*dx,cy+6.1f*dy,cx+8.1f*dx,cy+8.1f*dy,col,1.55f);
        }
    }
    void SmallGear(float cx, float cy, int id) {
        const unsigned col = 0x8A9296;
        brush->SetColor(C(col));
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},4.1f,4.1f), brush.Get(), 1.05f);
        rt->DrawEllipse(D2D1::Ellipse({cx,cy},1.25f,1.25f), brush.Get(), 1.0f);
        for (int i = 0; i < 6; ++i) {
            const float a = 6.2831853f * static_cast<float>(i) / 6.0f;
            const float dx = std::cos(a);
            const float dy = std::sin(a);
            Line(cx + 4.8f * dx, cy + 4.8f * dy,
                 cx + 6.0f * dx, cy + 6.0f * dy, col, 1.15f);
        }
        HitBox(cx - 9.0f, cy - 9.0f, cx + 9.0f, cy + 9.0f, id);
    }
    void NavButton(int idx, float y) {
        bool sel = page == idx;
        if (sel) {
            // Selected state uses only a slim indicator; no filled background.
            Round(D2D1::RectF(7.5f,y-7.0f,9.5f,y+7.0f), 1.0f, colors[405]);
        }
        unsigned col = sel ? 0xF1F3F4 : 0x737C82;
        float cx = 27, cy = y;
        switch (idx) {
            case 0: DrawAimbotIcon(cx,cy,col); break;
            case 1: DrawVisualsIcon(cx,cy,col); break;
            case 2: DrawMiscIcon(cx,cy,col); break;
            default: DrawSettingsIcon(cx,cy,col); break;
        }
        HitBox(7,y-14,47,y+14,7000+idx);
    }
    void DrawShell() {
        float bgA = 0.94f;
        Fill(D2D1::RectF(0,0,CW,CH), BeginnerCustomization::MainBackground, bgA);
        Fill(D2D1::RectF(0,0,SIDEBAR,CH), BeginnerCustomization::SidebarBackground, bgA);
        Line(SIDEBAR, 0, SIDEBAR, CH, BeginnerCustomization::DividerColour);
        RoundStroke(D2D1::RectF(.5f,.5f,CW-.5f,CH-.5f), WINDOW_RADIUS, BeginnerCustomization::WindowOutline);

        TriangleLogo(17,10);
        NavButton(0, 66);
        NavButton(1, 96);
        NavButton(2, 126);
        NavButton(3, 156);

        // Vector window controls: no font-dependent dash or multiplication glyph.
        const unsigned controlCol = 0x8E969A;
        Line(CW-40.0f, 16.0f, CW-32.0f, 16.0f, controlCol, 1.25f);
        Line(CW-20.0f, 12.0f, CW-12.0f, 20.0f, controlCol, 1.25f);
        Line(CW-12.0f, 12.0f, CW-20.0f, 20.0f, controlCol, 1.25f);
        HitBox(CW-45, 7, CW-27, 25, 7100);
        HitBox(CW-25, 7, CW-7, 25, 7101);
    }
    void Title(const wchar_t*) {
        // Page title intentionally hidden for the cleaner flat layout.
    }
    void Card(const wchar_t* title, float x, float y, float w, float h) {
        // Flat section layout: no nested card fill and no card border.
        if (drawingPage && x > 350.0f) w += CONTENT_SHIFT;
        Text(title, x+7, y+4, w-14, 16, 0x737B80, f8.Get());
        Line(x+7, y+23, x+w-7, y+23, 0x202528, 1.0f);
    }
    void Label(const wchar_t* t, float x, float y, unsigned col=0xA8AEB1, float w=150.f) {
        Text(t, x, y, w, 14, col, f9.Get());
    }
    void Checkbox(float x, float y, int id) {
        const D2D1_RECT_F r = D2D1::RectF(x,y,x+10,y+10);
        const bool on = checks[id];
        if (on) {
            Round(r, 2.0f, 0xE8EBEC);
            Line(x+2.2f,y+5.2f,x+4.2f,y+7.2f,0x202426,1.25f);
            Line(x+4.2f,y+7.2f,x+8.0f,y+2.8f,0x202426,1.25f);
        } else {
            RoundStroke(r, 2.0f, 0x596064, 1.0f);
        }
        HitBox(x-3,y-3,x+13,y+13,id);
    }
    void Toggle(float x, float y, int id) {
        const bool on = checks[id];
        if (on) Round(D2D1::RectF(x,y,x+24,y+12), 6.0f, colors[405]);
        else Round(D2D1::RectF(x,y,x+24,y+12), 6.0f, 0x2B3033);
        const float knobX = on ? x+18.0f : x+6.0f;
        Circle(knobX, y+6.0f, 4.15f, on ? 0x171A1C : 0x747B7F);
        HitBox(x-2,y-3,x+26,y+15,id);
    }
    void ColorBox(float x, float y, int id) {
        RGB c = colors[id];
        Round(D2D1::RectF(x,y,x+10,y+10), 2, c);
        RoundStroke(D2D1::RectF(x-.5f,y-.5f,x+10.5f,y+10.5f), 2, 0x545B5F);
        HitBox(x-3,y-3,x+13,y+13,1500+id);
    }
    void Slider(float x, float y, float w, int id) {
        float v = sliders[id];
        Line(x, y, x+w, y, 0x454B4E, 2);
        brush->SetColor(C(colors[405]));
        rt->DrawLine({x,y},{x+w*v,y},brush.Get(),2.0f);
        rt->FillEllipse(D2D1::Ellipse({x+w*v,y},2.4f,2.4f),brush.Get());
        HitBox(x,y-6,x+w,y+6,2500+id);
    }
    void Keybind(float x, float y, float w, int id) {
        Round(D2D1::RectF(x,y,x+w,y+13), 3, captureHotkey == id ? 0x343A3D : 0x24292B);
        RoundStroke(D2D1::RectF(x,y,x+w,y+13), 3, 0x3B4245);
        Text(captureHotkey == id ? L"press..." : hotkeys[id], x, y, w, 13, 0xC8CCCE, f8.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
        HitBox(x-2,y-2,x+w+2,y+15,3500+id);
    }
    void Dropdown(float x, float y, float w, int id, const std::vector<std::wstring>& items) {
        int idx = std::clamp(choices[id], 0, (int)items.size()-1);
        Round(D2D1::RectF(x,y,x+w,y+14), 3, 0x24292B);
        RoundStroke(D2D1::RectF(x,y,x+w,y+14), 3, 0x343A3D);
        Text(items[idx], x+5, y, w-18, 14, 0xBFC4C6, f8.Get());
        Line(x+w-13.0f,y+5.0f,x+w-9.0f,y+9.0f,0x747B7F,1.0f);
        Line(x+w-9.0f,y+9.0f,x+w-5.0f,y+5.0f,0x747B7F,1.0f);
        HitBox(x-2,y-2,x+w+2,y+16,4500+id);
    }
    void Button(const wchar_t* t, float x, float y, float w, float h, int id) {
        Round(D2D1::RectF(x,y,x+w,y+h), 3, 0x24292B);
        RoundStroke(D2D1::RectF(x,y,x+w,y+h), 3, 0x343A3D);
        Text(t, x, y, w, h, 0xC6CACC, f8.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
        HitBox(x,y,x+w,y+h,5500+id);
    }
    void RowCheck(const wchar_t* t, float x, float y, int id) {
        constexpr float controlCenter = 166.0f;
        Label(t, x, y); Checkbox(x + controlCenter - 5.0f, y+1, id);
    }
    void RowSettings(const wchar_t* t, float x, float y, int id) {
        constexpr float controlCenter = 166.0f;
        Label(t, x, y);
        SmallGear(x + controlCenter - 15.0f, y + 7.0f, id);
    }
    void RowToggle(const wchar_t* t, float x, float y, int id) {
        constexpr float controlCenter = 166.0f;
        Label(t, x, y); Toggle(x + controlCenter - 11.0f, y+1, id);
    }
    void RowColor(const wchar_t* t, float x, float y, int colorId, int checkId=-1) {
        constexpr float controlCenter = 166.0f;
        Label(t, x, y);
        ColorBox(x + controlCenter - 5.0f, y+2, colorId);
        if (checkId >= 0) Checkbox(x + controlCenter - 5.0f, y+1, checkId);
    }
    void Channel(const wchar_t* name, float x, float y, float w, int channel, float v) {
        Text(name, x, y, w, 13, 0xA8AEB1, f8.Get());
        Text(std::to_wstring((int)(v*255)), x+w-35, y, 35, 13, 0x777E82, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);
        float sy = y + 19;
        Line(x, sy, x+w, sy, 0x42484B, 4);
        Line(x, sy, x+w*v, sy, 0xE4E7E8, 4);
        Circle(x+w*v, sy, 3, 0xFFFFFF);
        HitBox(x, sy-7, x+w, sy+7, 7210+channel);
    }
    void DrawColorPopup() {
        if (openColor < 0) return;

        const float x = 323.0f;
        const float y = 54.0f;
        const float w = 224.0f;
        const float h = 258.0f;

        Round(D2D1::RectF(x, y, x + w, y + h), 6, 0x191D1F);
        RoundStroke(D2D1::RectF(x, y, x + w, y + h), 6, 0x3A4145);
        Text(L"Choose colour", x + 12, y + 7, 140, 18, 0xE5E7E8, fBold.Get());

        // Vector close icon instead of a text glyph.
        const float closeCx = x + w - 18.5f;
        const float closeCy = y + 14.5f;
        Line(closeCx - 3.5f, closeCy - 3.5f, closeCx + 3.5f, closeCy + 3.5f, 0xAEB4B7, 1.2f);
        Line(closeCx + 3.5f, closeCy - 3.5f, closeCx - 3.5f, closeCy + 3.5f, 0xAEB4B7, 1.2f);
        HitBox(x + w - 29, y + 4, x + w - 8, y + 25, 7999);

        RGB current = colors[openColor];
        float hue = 0.0f, saturation = 0.0f, value = 1.0f;
        RGBtoHSV(current, hue, saturation, value);

        const float svX = x + 12.0f;
        const float svY = y + 34.0f;
        const float svW = 166.0f;
        const float svH = 136.0f;
        const int columns = 28;
        const int rows = 22;

        // Visual saturation/value field.
        for (int row = 0; row < rows; ++row) {
            const float v0 = 1.0f - static_cast<float>(row) / static_cast<float>(rows);
            const float v1 = 1.0f - static_cast<float>(row + 1) / static_cast<float>(rows);
            const float cellTop = svY + svH * static_cast<float>(row) / static_cast<float>(rows);
            const float cellBottom = svY + svH * static_cast<float>(row + 1) / static_cast<float>(rows);

            for (int column = 0; column < columns; ++column) {
                const float s0 = static_cast<float>(column) / static_cast<float>(columns);
                const float s1 = static_cast<float>(column + 1) / static_cast<float>(columns);
                const RGB colour = HSVtoRGB(hue, (s0 + s1) * 0.5f, (v0 + v1) * 0.5f);
                const float cellLeft = svX + svW * static_cast<float>(column) / static_cast<float>(columns);
                const float cellRight = svX + svW * static_cast<float>(column + 1) / static_cast<float>(columns);
                Fill(D2D1::RectF(cellLeft, cellTop, cellRight + 0.4f, cellBottom + 0.4f), colour);
            }
        }
        RoundStroke(D2D1::RectF(svX - 0.5f, svY - 0.5f, svX + svW + 0.5f, svY + svH + 0.5f), 3, 0x4B5357);

        const float markerX = svX + saturation * svW;
        const float markerY = svY + (1.0f - value) * svH;
        brush->SetColor(C(0x000000));
        rt->DrawEllipse(D2D1::Ellipse({markerX, markerY}, 5.0f, 5.0f), brush.Get(), 2.7f);
        brush->SetColor(C(0xFFFFFF));
        rt->DrawEllipse(D2D1::Ellipse({markerX, markerY}, 4.0f, 4.0f), brush.Get(), 1.4f);
        HitBox(svX, svY, svX + svW, svY + svH, 7216);

        // Vertical hue strip.
        const float hueX = x + 187.0f;
        const float hueY = svY;
        const float hueW = 18.0f;
        const float hueH = svH;
        const int hueSteps = 48;
        for (int step = 0; step < hueSteps; ++step) {
            const float h0 = static_cast<float>(step) / static_cast<float>(hueSteps);
            const RGB colour = HSVtoRGB(h0, 1.0f, 1.0f);
            const float top = hueY + hueH * static_cast<float>(step) / static_cast<float>(hueSteps);
            const float bottom = hueY + hueH * static_cast<float>(step + 1) / static_cast<float>(hueSteps);
            Fill(D2D1::RectF(hueX, top, hueX + hueW, bottom + 0.4f), colour);
        }
        RoundStroke(D2D1::RectF(hueX - 0.5f, hueY - 0.5f, hueX + hueW + 0.5f, hueY + hueH + 0.5f), 3, 0x4B5357);
        const float hueMarkerY = hueY + hue * hueH;
        Fill(D2D1::RectF(hueX - 3.0f, hueMarkerY - 1.5f, hueX + hueW + 3.0f, hueMarkerY + 1.5f), 0xFFFFFF);
        Stroke(D2D1::RectF(hueX - 3.0f, hueMarkerY - 2.5f, hueX + hueW + 3.0f, hueMarkerY + 2.5f), 0x111416, 1.0f);
        HitBox(hueX - 3.0f, hueY, hueX + hueW + 3.0f, hueY + hueH, 7217);

        Text(L"Presets", x + 12, y + 179, 70, 14, 0x858C90, f8.Get());
        const unsigned presets[] = {
            0xF05A5A, 0xF4A62A, 0xF3DD4F, 0x53D76A,
            0x43BEEA, 0x9B62E8, 0xE164C7
        };
        for (int i = 0; i < 7; ++i) {
            Round(D2D1::RectF(x + 12 + i * 27, y + 197, x + 31 + i * 27, y + 216), 3, presets[i]);
            HitBox(x + 12 + i * 27, y + 197, x + 31 + i * 27, y + 216, 7300 + i);
        }

        Text(L"Selected", x + 12, y + 226, 55, 16, 0x858C90, f8.Get());
        Round(D2D1::RectF(x + 70, y + 227, x + 205, y + 244), 3, current);
        RoundStroke(D2D1::RectF(x + 69.5f, y + 226.5f, x + 205.5f, y + 244.5f), 3, 0x4B5357);
    }
    void DrawDropdownPopup() {
        if (openDropdown < 0) return;
        float x = dropdownAnchor.left;
        float y = dropdownAnchor.bottom + 3;
        float w = dropdownAnchor.right - dropdownAnchor.left;
        float h = 18.f * (float)dropdownItems.size();
        Round(D2D1::RectF(x,y,x+w,y+h), 4, 0x1C2022);
        RoundStroke(D2D1::RectF(x,y,x+w,y+h), 4, 0x3A4145);
        for (size_t i=0;i<dropdownItems.size();i++) {
            if ((int)i == choices[openDropdown]) Fill(D2D1::RectF(x+2, y+2+i*18, x+w-2, y+17+i*18), 0x2A3033);
            Text(dropdownItems[i], x+6, y+i*18, w-12, 18, 0xC6CACC, f8.Get());
            HitBox(x, y+i*18, x+w, y+(i+1)*18, 7400+(int)i);
        }
    }

void Paint() {
        if (FAILED(EnsureRT())) return;
        hits.clear();
        rt->BeginDraw();
        rt->SetTransform(D2D1::Matrix3x2F::Identity());
        rt->Clear(C(BeginnerCustomization::MainBackground, 1.0f));
        DrawShell();

        drawingPage = true;
        const float pageScrollY = page == 1 ? -visualsScroll : 0.0f;
        rt->SetTransform(D2D1::Matrix3x2F::Translation(-CONTENT_SHIFT, pageScrollY));

        if (page == 0) DrawAimbot();
        else if (page == 1) DrawVisuals();
        else if (page == 2) DrawMisc();
        else DrawSettings();

        drawingPage = false;
        rt->SetTransform(D2D1::Matrix3x2F::Identity());
        DrawVisualsScrollbar();
        DrawVisualSettingsPopup();
        DrawDropdownPopup();
        DrawColorPopup();

        HRESULT hr = rt->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) { rt.Reset(); brush.Reset(); }
    }
