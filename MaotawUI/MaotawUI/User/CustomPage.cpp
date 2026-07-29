#include "../Core/App.h"

// Central page router for every sidebar page except Visuals (page 0).
// Keep built-in pages routed here, then add future custom pages below.
void App::DrawOtherPage(int page)
{
    switch (page)
    {
    case 1:
        DrawSettingsPage();
        return;

    case 3:
        DrawProfilePage();
        return;

    default:
        break;
    }

    DrawText(L"Custom page", Layout::LeftX, 20.0f, Layout::RightRight, 48.0f,
        Theme::Text, text_);
    DrawText(L"Edit User/CustomPage.cpp to add your own UI", Layout::LeftX, 20.0f,
        Layout::RightRight, 48.0f, Theme::SubText, small_, DWRITE_TEXT_ALIGNMENT_TRAILING);
    Divider(Layout::LeftX, Layout::RightRight, 61.0f);
    DrawText(L"YOUR LOGIC", Layout::LeftX, 76.0f, Layout::RightRight, 96.0f,
        Theme::Muted, tiny_);
    DrawText(L"Selected custom page: " + std::to_wstring(page),
        Layout::LeftX, 108.0f, Layout::RightRight, 138.0f, Theme::SubText, small_);
}
