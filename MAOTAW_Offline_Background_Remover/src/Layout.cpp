#include "App.h"

D2D1_RECT_F MainButton(float w)
{
    float cx = w * 0.5f;
    return { cx - 120, 460, cx + 120, 500 };
}

D2D1_RECT_F ChangeButton()
{
    return { 28, 518, 192, 554 };
}

D2D1_RECT_F SaveButton(float w)
{
    return { w - 188, 518, w - 28, 554 };
}

D2D1_RECT_F EraserButtonRect(float w)
{
    return { w - 74, 22, w - 50, 46 };
}

D2D1_RECT_F SettingsButtonRect(float w)
{
    return { w - 42, 22, w - 18, 46 };
}

D2D1_RECT_F SettingsPopupRect(float w)
{
    return { w - 252, 56, w - 24, 176 };
}

D2D1_RECT_F FeatherTrackRect(float w)
{
    D2D1_RECT_F p = SettingsPopupRect(w);
    return { p.left + 18, p.top + 74, p.right - 18, p.top + 80 };
}

D2D1_RECT_F ChooseCardRect(float w)
{
    return { 150, 118, w - 150, 404 };
}

D2D1_RECT_F ChooseSelectButtonRect(float w)
{
    float cx = w * 0.5f;
    return { cx - 102, 338, cx + 102, 376 };
}

D2D1_RECT_F PreviewCardRect(float w)
{
    float cardW = w - 120.0f;
    float left = (w - cardW) * 0.5f;
    return { left, 96, left + cardW, 436 };
}

D2D1_RECT_F CurrentPreviewImageRect(float w)
{
    D2D1_RECT_F card = PreviewCardRect(w);
    D2D1_RECT_F area = { card.left, card.top + 34, card.right, card.bottom };
    ID2D1Bitmap* b = g_outputBitmap ? g_outputBitmap : g_inputBitmap;
    return CalcFitRect(b, area);
}
