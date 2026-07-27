#pragma once

// Draw.cpp owns every Direct2D drawing function:
// - sidebar and navigation icons
// - Home, Library, Favorites and Settings pages
// - cards, previews, dropdown, scrollbar and hover states
//
// Called by Window.cpp during WM_PAINT.
static void DrawWindow();
