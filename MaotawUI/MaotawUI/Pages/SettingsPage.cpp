#include "../Core/App.h"


void App::DrawConnectedParticles(float width, float height)
{
    if (!settings_.connectedParticles)
        return;

    constexpr std::size_t maximumParticleCount = 48;
    const std::size_t particleCount = static_cast<std::size_t>(
        std::clamp(static_cast<int>(std::round(settings_.particleCount)), 8, 48));
    const float connectionDistance = Clamp(settings_.connectionDistance, 40.0f, 180.0f);
    const float opacity = Clamp(settings_.particleOpacity / 100.0f, 0.05f, 1.0f);
    std::array<D2D1_POINT_2F, maximumParticleCount> points{};

    const float time = static_cast<float>(GetTickCount64() % 1000000ULL) / 1000.0f;
    const float speed = settings_.particleSpeed / 22.0f;
    const float left = Layout::Sidebar + 9.0f;
    const float usableWidth = std::max(1.0f, width - left - 10.0f);
    const float usableHeight = std::max(1.0f, height - 20.0f);

    for (std::size_t index = 0; index < particleCount; ++index)
    {
        const float seed = static_cast<float>(index);
        const float baseX = std::fmod(seed * 87.13f + 19.0f, usableWidth);
        const float baseY = std::fmod(seed * 53.71f + 31.0f, usableHeight);
        const float driftX = time * speed * (2.2f + std::fmod(seed * 1.91f, 4.8f));
        const float driftY = time * speed * (1.2f + std::fmod(seed * 1.37f, 3.2f));

        points[index].x = left + std::fmod(
            baseX + driftX + std::sin(time * 0.43f + seed) * 10.0f + usableWidth,
            usableWidth);
        points[index].y = 10.0f + std::fmod(
            baseY + driftY + std::cos(time * 0.37f + seed * 0.7f) * 8.0f + usableHeight,
            usableHeight);
    }

    for (std::size_t first = 0; first < particleCount; ++first)
    {
        for (std::size_t second = first + 1; second < particleCount; ++second)
        {
            const float dx = points[first].x - points[second].x;
            const float dy = points[first].y - points[second].y;
            const float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < connectionDistance)
            {
                const float alpha = opacity * 0.27f * (1.0f - distance / connectionDistance);
                DrawLine(points[first].x, points[first].y,
                    points[second].x, points[second].y,
                    WithAlpha(settings_.particleColor, alpha), 0.8f);
            }
        }
    }

    SetBrush(WithAlpha(settings_.particleColor, opacity));
    for (std::size_t index = 0; index < particleCount; ++index)
    {
        renderTarget_->FillEllipse(
            D2D1::Ellipse(points[index], 1.25f, 1.25f),
            brush_);
    }
}

void App::DrawSettingsPage()
{
    DrawText(L"Settings", Layout::LeftX, 20.0f, Layout::RightRight, 48.0f,
        Theme::Text, text_);
    DrawText(L"Customise the window and animated background", Layout::LeftX, 20.0f,
        Layout::RightRight, 48.0f, Theme::SubText, small_, DWRITE_TEXT_ALIGNMENT_TRAILING);
    Divider(Layout::LeftX, Layout::RightRight, 61.0f);

    const float leftColumnLeft = Layout::LeftX;
    const float leftColumnRight = Layout::SplitX - 18.0f;
    const float rightColumnLeft = Layout::SplitX + 18.0f;
    const float rightColumnRight = Layout::RightRight;

    DrawText(L"ANIMATED BACKGROUND", leftColumnLeft, 72.0f, leftColumnRight, 92.0f,
        Theme::Muted, tiny_);

    ToggleRow(L"Connected particles", settings_.connectedParticles,
        leftColumnLeft, leftColumnRight, 94.0f);

    SliderControl(L"Particle speed", settings_.particleSpeed, 0.0f, 60.0f,
        leftColumnLeft, leftColumnRight, 137.0f, ActiveSlider::ParticleSpeed, 0);

    SliderControl(L"Particle density", settings_.particleCount, 8.0f, 48.0f,
        leftColumnLeft, leftColumnRight, 190.0f, ActiveSlider::ParticleCount, 0);

    SliderControl(L"Connection range", settings_.connectionDistance, 40.0f, 180.0f,
        leftColumnLeft, leftColumnRight, 243.0f, ActiveSlider::ConnectionDistance, 0);

    SliderControl(L"Particle opacity", settings_.particleOpacity, 5.0f, 100.0f,
        leftColumnLeft, leftColumnRight, 296.0f, ActiveSlider::ParticleOpacity, 0);

    ColorRow(L"Particle color", settings_.particleColor,
        leftColumnLeft, leftColumnRight, 354.0f);

    DrawText(L"WINDOW CUSTOMISATION", rightColumnLeft, 72.0f, rightColumnRight, 92.0f,
        Theme::Muted, tiny_);

    SliderControl(L"Window transparency", settings_.windowOpacity, 25.0f, 100.0f,
        rightColumnLeft, rightColumnRight, 94.0f, ActiveSlider::WindowOpacity, 0);

    ToggleRow(L"Window outline", settings_.windowBorder,
        rightColumnLeft, rightColumnRight, 151.0f);

    SliderControl(L"Outline transparency", settings_.windowBorderOpacity, 0.0f, 100.0f,
        rightColumnLeft, rightColumnRight, 194.0f, ActiveSlider::WindowBorderOpacity, 0);

    ToggleRow(L"Enable scroller", settings_.enableScroller,
        rightColumnLeft, rightColumnRight, 251.0f);

    ColorRow(L"Accent color", settings_.accentColor,
        rightColumnLeft, rightColumnRight, 294.0f);

    Divider(rightColumnLeft, rightColumnRight, 348.0f);
    DrawText(L"Window and outline transparency update immediately and are saved in configs.",
        rightColumnLeft, 360.0f, rightColumnRight, 398.0f,
        Theme::SubText, tiny_);
}
