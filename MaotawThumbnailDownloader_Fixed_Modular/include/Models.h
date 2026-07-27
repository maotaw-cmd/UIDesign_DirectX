#pragma once

#include <string>
#include <d2d1.h>

// Data shown by Library and Favorites pages.
struct LibraryItem
{
    std::wstring path;
    std::wstring title;
    bool favorite = false;
    ID2D1Bitmap* bitmap = nullptr;
};
