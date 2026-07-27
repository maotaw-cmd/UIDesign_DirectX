// Included inside class App. Edit this file to customise this page.

    void DrawAimbot() {
        Title(L"Aimbot");

        Card(L"General", 153, 34, 194, 150);
        RowToggle(L"Enable aimbot", 160, 64, 1);
        Label(L"Target bone", 160, 92); Dropdown(258, 92, 78, 100, {L"Head",L"Neck",L"Chest",L"Nearest"});
        Label(L"Aimbot key", 160, 120); Keybind(303, 120, 33, 300);
        RowCheck(L"Visibility check", 160, 148, 2);
        RowCheck(L"Autowall", 160, 172, 3);

        Card(L"Smoothing / FOV", 153, 194, 194, 161);
        Label(L"Smooth X", 160, 218); Text(std::to_wstring((int)(sliders[200]*100)), 304, 218, 32, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(160, 238, 176, 200);
        Label(L"Smooth Y", 160, 255); Text(std::to_wstring((int)(sliders[201]*100)), 304, 255, 32, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(160, 275, 176, 201);
        Label(L"Field of view", 160, 292); Text(std::to_wstring((int)(sliders[202]*180)), 304, 292, 32, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(160, 312, 176, 202);
        Label(L"FOV circle", 160, 330); ColorBox(310, 332, 400); Checkbox(326, 331, 4);

        Card(L"Triggerbot / Recoil", 357, 34, 197, 321);
        RowToggle(L"Triggerbot", 364, 64, 5);
        Label(L"Trigger key", 364, 94); Keybind(511, 94, 33, 301);
        Label(L"Hitchance", 364, 124); Text(std::to_wstring((int)(sliders[203]*100))+L"%", 509, 124, 35, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(364, 145, 180, 203);
        Label(L"First bullet delay", 364, 172); Text(std::to_wstring((int)(sliders[204]*120))+L"ms", 509, 172, 35, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(364, 193, 180, 204);
        RowToggle(L"Recoil compensation", 364, 224, 6);
        RowCheck(L"Link recoil to aimbot", 364, 252, 7);
        Label(L"Behaviour", 364, 286, 0x7D8488, 170);
        Text(L"Natural smoothing with separate X / Y", 364, 304, 178, 13, 0x767D81, f8.Get());
        Text(L"speed, FOV limiting and trigger delay.", 364, 319, 178, 13, 0x767D81, f8.Get());
    }
