// Included inside class App. Edit this file to customise this page.

    void VisualTargetButton(const wchar_t* label, float x, int target) {
        const bool selected = visualTarget == target;
        if (selected)
            Round(D2D1::RectF(x, 30, x + 72, 50), 3.0f, 0x2A3033);
        else
            Round(D2D1::RectF(x, 30, x + 72, 50), 3.0f, 0x1C2022);
        RoundStroke(D2D1::RectF(x, 30, x + 72, 50), 3.0f,
                    selected ? 0x596166 : 0x303638);
        Text(label, x, 30, 72, 20,
             selected ? 0xF0F2F3 : 0x8A9296,
             f8.Get(), DWRITE_TEXT_ALIGNMENT_CENTER,
             DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        HitBox(x, 30, x + 72, 50, 8100 + target);
    }
    void DrawVisuals() {
        VisualTargetButton(L"Enemy", 153, 0);
        VisualTargetButton(L"Local", 231, 1);

        const bool local = visualTarget == 1;
        const int enableId = local ? 50 : 10;
        const int skeletonId = local ? 51 : 11;
        const int nameId = local ? 52 : 12;
        const int healthId = local ? 53 : 13;
        const int armorId = local ? 54 : 14;
        const int weaponId = local ? 55 : 15;
        const int distanceId = local ? 56 : 16;
        const int snaplineId = local ? 57 : 18;
        const int glowId = local ? 58 : 17;
        const int chamsId = local ? 59 : 21;
        const int snapColourId = local ? 410 : 403;

        Card(local ? L"Local player" : L"Enemy player", 153, 58, 194, 138);
        RowToggle(local ? L"Enable local" : L"Enable enemy", 160, 86, enableId);
        RowSettings(L"Box preview", 160, 112, 8000);
        RowCheck(L"Skeleton", 160, 138, skeletonId);
        RowCheck(L"Name", 160, 164, nameId);

        Card(L"Information", 153, 190, 194, 165);
        RowCheck(L"Health bar", 160, 220, healthId);
        SmallGear(307, 227, 8003);
        RowCheck(L"Armor bar", 160, 248, armorId);
        SmallGear(307, 255, 8004);
        RowCheck(L"Weapon", 160, 276, weaponId);
        RowCheck(L"Distance", 160, 304, distanceId);

        Card(L"Model effects", 357, 58, 197, 138);
        RowToggle(L"Glow", 364, 86, glowId);
        SmallGear(507, 93, 8001);
        RowToggle(L"Character chams", 364, 114, chamsId);
        SmallGear(507, 121, 8002);
        RowToggle(L"Snaplines", 364, 142, snaplineId);
        Label(L"Snapline colour", 364, 168);
        ColorBox(525, 170, snapColourId);

        Card(L"Preview target", 357, 208, 197, 147);
    }
    void DrawVisualSettingsPopup() {
        if (openVisualPopup < 0) return;

        const float x = 286.0f;
        const float y = 62.0f;
        const float w = 258.0f;
        float h = 176.0f;
        if (openVisualPopup == 0) h = 146.0f;
        else if (openVisualPopup == 3 || openVisualPopup == 4) h = 210.0f;

        Round(D2D1::RectF(x, y, x + w, y + h), 6.0f, 0x191D1F);
        RoundStroke(D2D1::RectF(x, y, x + w, y + h), 6.0f, 0x3A4145, 1.0f);

        const wchar_t* title = L"Visual settings";
        if (openVisualPopup == 0)
            title = visualTarget == 1 ? L"Local box settings" : L"Enemy box settings";
        else if (openVisualPopup == 1) title = L"Glow settings";
        else if (openVisualPopup == 2) title = L"Chams settings";
        else if (openVisualPopup == 3) title = L"Health-bar settings";
        else if (openVisualPopup == 4) title = L"Armor-bar settings";

        Text(title, x + 13, y + 7, 190, 18, 0xE5E7E8, fBold.Get());
        Line(x + w - 24, y + 10, x + w - 14, y + 20, 0xAAB0B3, 1.2f);
        Line(x + w - 14, y + 10, x + w - 24, y + 20, 0xAAB0B3, 1.2f);
        HitBox(x + w - 31, y + 4, x + w - 7, y + 27, 8099);

        if (openVisualPopup == 0) {
            const bool local = visualTarget == 1;
            const int styleId = local ? 121 : 101;
            const int boxColourId = local ? 402 : 401;
            const int filledColourId = local ? 417 : 416;

            Label(L"Box style", x + 14, y + 42);
            Dropdown(x + 112, y + 42, 128, styleId,
                     {L"Cornered",L"Box",L"Box + Filled",L"Cornered + Filled"});

            Label(L"Box colour", x + 14, y + 76);
            ColorBox(x + 226, y + 78, boxColourId);

            Label(L"Filled colour", x + 14, y + 106);
            ColorBox(x + 226, y + 108, filledColourId);
            return;
        }

        if (openVisualPopup == 1) {
            const bool local = visualTarget == 1;
            const int styleId = local ? 122 : 103;
            const int thicknessId = local ? 215 : 214;
            const int glowColourId = local ? 419 : 418;
            Label(L"Glow style", x + 14, y + 42);
            Dropdown(x + 112, y + 42, 128, styleId,
                     {L"Soft",L"Pulse",L"Dynamic",L"Rainbow",L"Neon"});
            Label(L"Thickness", x + 14, y + 78);
            wchar_t value[32]{};
            swprintf_s(value, L"%.1f px", 0.5f + sliders[thicknessId] * 2.0f);
            Text(value, x + 190, y + 78, 50, 14, 0x9AA1A5, f8.Get(),
                 DWRITE_TEXT_ALIGNMENT_TRAILING);
            Slider(x + 112, y + 103, 128, thicknessId);
            Label(L"Glow colour", x + 14, y + 130);
            ColorBox(x + 226, y + 132, glowColourId);
        } else if (openVisualPopup == 2) {
            const bool local = visualTarget == 1;
            const int styleId = local ? 123 : 107;
            const int colourId = local ? 415 : 406;
            Label(L"Chams style", x + 14, y + 42);
            Dropdown(x + 112, y + 42, 128, styleId,
                     {L"Solid",L"Pulse",L"Rainbow",L"Metallic"});
            Label(L"Character colour", x + 14, y + 80);
            ColorBox(x + 226, y + 82, colourId);
            Text(L"Changes update on the 3D model live.", x + 14, y + 118,
                 220, 14, 0x767D81, f8.Get());
        } else {
            const bool health = openVisualPopup == 3;
            const bool local = visualTarget == 1;
            const int styleId = local ? (health ? 124 : 125) : (health ? 105 : 106);
            const int widthId = local ? (health ? 216 : 217) : (health ? 212 : 213);
            const int foreId = local ? (health ? 411 : 413) : (health ? 404 : 408);
            const int backId = local ? (health ? 412 : 414) : (health ? 407 : 409);
            const int positionId = local ? (health ? 126 : 127) : (health ? 108 : 109);
            Label(L"Position", x + 14, y + 42);
            Dropdown(x + 112, y + 42, 128, positionId,
                     {L"Left",L"Right",L"Top",L"Bottom"});
            Label(L"Style", x + 14, y + 74);
            Dropdown(x + 112, y + 74, 128, styleId,
                     {L"Normal",L"Segmented",L"Gradient"});
            Label(L"Width", x + 14, y + 108);
            Text(std::to_wstring((int)(1.0f + sliders[widthId] * 11.0f)) + L" px",
                 x + 190, y + 108, 50, 14, 0x9AA1A5, f8.Get(),
                 DWRITE_TEXT_ALIGNMENT_TRAILING);
            Slider(x + 112, y + 133, 128, widthId);
            Label(L"Foreground", x + 14, y + 158);
            ColorBox(x + 226, y + 160, foreId);
            Label(L"Background", x + 14, y + 181);
            ColorBox(x + 226, y + 183, backId);
        }
    }
    void DrawVisualsScrollbar() {
        if (page != 1) return;

        const float viewportHeight = VisualViewportBottom - VisualViewportTop;
        const float maximumScroll = std::max(0.0f, VisualContentHeight - viewportHeight);
        if (maximumScroll <= 0.0f) return;

        const float trackX = CW - 4.0f;
        const float trackTop = VisualViewportTop + 3.0f;
        const float trackBottom = VisualViewportBottom - 3.0f;
        const float trackHeight = trackBottom - trackTop;
        const float thumbHeight = std::max(42.0f, trackHeight * (viewportHeight / VisualContentHeight));
        const float available = trackHeight - thumbHeight;
        const float ratio = visualsScroll / maximumScroll;
        const float thumbTop = trackTop + available * ratio;

        Fill(D2D1::RectF(trackX, trackTop, trackX + 1.5f, trackBottom), 0x31373A, 0.35f);
        Round(D2D1::RectF(trackX, thumbTop, trackX + 1.5f, thumbTop + thumbHeight),
              0.75f, 0xF2F4F5, 0.92f);
        HitBox(trackX - 4.0f, trackTop, trackX + 5.0f, trackBottom, 7900);
    }
    void SetVisualScroll(float value) {
        const float viewportHeight = VisualViewportBottom - VisualViewportTop;
        const float maximumScroll = std::max(0.0f, VisualContentHeight - viewportHeight);
        visualsScroll = std::clamp(value, 0.0f, maximumScroll);
        openDropdown = -1;
        openColor = -1;
        InvalidateRect(hwnd, nullptr, FALSE);
    }
    void UpdateVisualScrollbarFromMouse(float mouseY) {
        const float trackTop = VisualViewportTop + 3.0f;
        const float trackBottom = VisualViewportBottom - 3.0f;
        const float trackHeight = trackBottom - trackTop;
        const float viewportHeight = VisualViewportBottom - VisualViewportTop;
        const float maximumScroll = std::max(0.0f, VisualContentHeight - viewportHeight);
        const float thumbHeight = std::max(42.0f, trackHeight * (viewportHeight / VisualContentHeight));
        const float available = std::max(1.0f, trackHeight - thumbHeight);
        const float thumbTop = std::clamp(mouseY - visualScrollbarDragOffset,
                                          trackTop, trackTop + available);
        SetVisualScroll(((thumbTop - trackTop) / available) * maximumScroll);
    }
