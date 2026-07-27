// Included inside class App. Edit this file to customise this page.

    void DrawMisc() {
        Title(L"Miscellaneous");

        Card(L"Skin changer", 153, 34, 194, 118);
        RowToggle(L"Enable", 160, 64, 30);
        Label(L"Knife", 160, 80); Dropdown(278, 80, 58, 110, {L"Bayonet",L"Karambit",L"Butterfly",L"M9"});
        Label(L"Gloves", 160, 102); Dropdown(278, 102, 58, 111, {L"Sport",L"Driver",L"Moto",L"Wraps"});
        Text(L"Access to all knives, gloves and items.", 160, 126, 170, 13, 0x767D81, f8.Get());

        Card(L"World / movement", 153, 161, 194, 194);
        RowCheck(L"No flash", 160, 185, 31);
        RowCheck(L"No smoke", 160, 206, 32);
        RowToggle(L"Bunny hop", 160, 227, 33);
        Label(L"Jump key", 160, 249); Keybind(305, 249, 31, 302);
        RowToggle(L"Third person", 160, 272, 34);
        Label(L"Third person key", 160, 294); Keybind(305, 294, 31, 303);
        Label(L"Camera distance", 160, 318); Text(std::to_wstring((int)(sliders[220]*300)), 304, 318, 32, 14, 0x858C90, f8.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING); Slider(160, 337, 176, 220);

        Card(L"Bomb / C4 ESP", 357, 34, 197, 140);
        RowToggle(L"Enable C4 ESP", 364, 64, 35);
        RowCheck(L"Defuse timer", 364, 80, 36);
        RowCheck(L"Explosion countdown", 364, 101, 37);
        Text(L"Show planted bomb information directly", 364, 126, 178, 13, 0x767D81, f8.Get());
        Text(L"on screen with timer indicators.", 364, 139, 178, 13, 0x767D81, f8.Get());

        Card(L"Extra", 357, 183, 197, 172);
        Text(L"All controls here are UI only and can be", 364, 208, 175, 13, 0x767D81, f8.Get());
        Text(L"extended with your own back-end logic.", 364, 221, 175, 13, 0x767D81, f8.Get());
        Label(L"Menu key", 364, 255); Keybind(511, 255, 33, 304);
        RowCheck(L"Animations", 364, 280, 43);
        RowCheck(L"Tooltips", 364, 301, 44);
    }
