#include "../Core/App.h"
#include "../User/UserLogic.h"

LRESULT CALLBACK App::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    App* app = nullptr;

    if (message == WM_NCCREATE)
    {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        app->hwnd_ = hwnd;
    }
    else
    {
        app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    return app ? app->WindowProc(message, wParam, lParam)
        : DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT App::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        // Do not rebuild the full Direct2D scene while Windows is moving the
        // top-level window. Windows can move the already-presented surface,
        // which is much smoother than rendering the textured model on every
        // intermediate move message.
        if (!windowMoveActive_)
        {
            UserLogic::OnFrame(*this);

            // Only animate pages that contain moving content. Static pages are
            // redrawn by their own input/events instead of wasting a full frame.
            const bool visualsAnimating = selectedPage_ == 0;
            const bool particlesAnimating = selectedPage_ == 1 && settings_.connectedParticles;
            const bool transientAnimating = statusTimer_ > 0.0f;
            if (visualsAnimating || particlesAnimating || transientAnimating)
                InvalidateRect(hwnd_, nullptr, FALSE);
        }
        return 0;

    case WM_ENTERSIZEMOVE:
        windowMoveActive_ = true;
        KillTimer(hwnd_, 1);
        // Ensure Windows has one fresh frame to move as a cached surface.
        RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
        return 0;

    case WM_EXITSIZEMOVE:
        windowMoveActive_ = false;
        SetTimer(hwnd_, 1, 33, nullptr);
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_NCHITTEST:
    {
        const LRESULT defaultResult = DefWindowProcW(hwnd_, message, wParam, lParam);
        if (defaultResult != HTCLIENT)
            return defaultResult;

        POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd_, &point);
        if (point.y >= 0 && point.y < static_cast<LONG>(Layout::DragStrip))
            return HTCAPTION;
        return HTCLIENT;
    }

    case WM_MOUSEMOVE:
    {
        mouse_.x = GET_X_LPARAM(lParam);
        mouse_.y = GET_Y_LPARAM(lParam);

        TRACKMOUSEEVENT tracking{};
        tracking.cbSize = sizeof(tracking);
        tracking.dwFlags = TME_LEAVE;
        tracking.hwndTrack = hwnd_;
        TrackMouseEvent(&tracking);

        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;
    }

    case WM_MOUSELEAVE:
        mouse_ = { -1000, -1000 };
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
    {
        mouse_.x = GET_X_LPARAM(lParam);
        mouse_.y = GET_Y_LPARAM(lParam);

        // Handle built-in sidebar navigation before any popup/card input.
        // This guarantees that a stale Local/Enemy selector, combo box, or
        // settings card can never steal the Settings icon click.
        RECT client{};
        GetClientRect(hwnd_, &client);
        const float clientHeight = static_cast<float>(client.bottom - client.top);
        const bool settingsIconHit = Hit(
            static_cast<float>(mouse_.x), static_cast<float>(mouse_.y),
            14.0f, clientHeight - 61.0f, 58.0f, clientHeight - 18.0f);

        if (settingsIconHit)
        {
            selectedPage_ = 1;
            openCombo_ = 0;
            targetPopupOpen_ = false;
            visualSettingsPopup_ = 0;
            configPopupOpen_ = false;
            colorPickerOpen_ = false;
            pickerTarget_ = nullptr;
            configNameFocused_ = false;
            activeSlider_ = ActiveSlider::None;
            activeColorArea_ = -1;
            mouseDown_ = false;
            clicked_ = false;
            clickConsumed_ = true;
            SetFocus(hwnd_);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        // Handle the Visuals profile selector and every option gear directly
        // from WM_LBUTTONDOWN. These controls previously depended on a render
        // frame receiving clicked_, so switching Local/Enemy could leave a
        // modal state that swallowed the next gear click.
        if (selectedPage_ == 0 && !configPopupOpen_ && !colorPickerOpen_)
        {
            const float mx = static_cast<float>(mouse_.x);
            const float my = static_cast<float>(mouse_.y);

            // Local / Enemy popup owns all clicks while it is open.
            if (targetPopupOpen_)
            {
                const float popupRight = Layout::LeftRight;
                const float popupLeft = popupRight - 154.0f;
                const float popupTop = 48.0f;

                for (int item = 0; item < 2; ++item)
                {
                    const float itemTop = popupTop + 4.0f + item * 28.0f;
                    if (Hit(mx, my, popupLeft + 4.0f, itemTop,
                        popupRight - 4.0f, itemTop + 26.0f))
                    {
                        selectedVisualTarget_ = item;
                        targetPopupOpen_ = false;
                        visualSettingsPopup_ = 0;
                        openCombo_ = 0;
                        activeSlider_ = ActiveSlider::None;
                        clicked_ = false;
                        mouseDown_ = false;
                        clickConsumed_ = true;
                        InvalidateRect(hwnd_, nullptr, FALSE);
                        return 0;
                    }
                }

                // An outside click only closes the selector. It must never
                // activate a row or gear underneath on the same click.
                targetPopupOpen_ = false;
                clicked_ = false;
                mouseDown_ = false;
                clickConsumed_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }

            // Profile icon at the top-right of the Visuals column.
            const float profileLeft = Layout::LeftRight - 34.0f;
            if (Hit(mx, my, profileLeft, 14.0f, Layout::LeftRight, 46.0f))
            {
                targetPopupOpen_ = true;
                visualSettingsPopup_ = 0;
                openCombo_ = 0;
                activeSlider_ = ActiveSlider::None;
                clicked_ = false;
                mouseDown_ = false;
                clickConsumed_ = true;
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }

            // Gear icon beside each Visual option.
            constexpr float firstRowY = 64.0f;
            constexpr float rowStep = 27.0f;
            constexpr float rowHeight = 26.0f;
            const float gearLeft = Layout::LeftRight - 25.0f;

            for (int id = 1; id <= 14; ++id)
            {
                const float rowY = firstRowY + rowStep * static_cast<float>(id - 1);
                if (Hit(mx, my, gearLeft, rowY, Layout::LeftRight, rowY + rowHeight))
                {
                    visualSettingsPopup_ = (visualSettingsPopup_ == id) ? 0 : id;
                    if (visualSettingsPopup_ != 0)
                    {
                        visualPopupX_ = gearLeft - 10.0f;
                        visualPopupY_ = rowY + rowHeight + 4.0f;
                    }
                    openCombo_ = 0;
                    targetPopupOpen_ = false;
                    activeSlider_ = ActiveSlider::None;
                    clicked_ = false;
                    mouseDown_ = false;
                    clickConsumed_ = true;
                    SetFocus(hwnd_);
                    InvalidateRect(hwnd_, nullptr, FALSE);
                    return 0;
                }
            }
        }

        SetCapture(hwnd_);
        mouseDown_ = true;
        clicked_ = true;
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;
    }

    case WM_LBUTTONUP:
        mouseDown_ = false;
        activeSlider_ = ActiveSlider::None;
        activeColorArea_ = -1;
        configScrollbarDragging_ = false;
        visualPopupDragging_ = false;
        configPopupDragging_ = false;
        ReleaseCapture();
        InvalidateRect(hwnd_, nullptr, FALSE);
        return 0;

    case WM_MOUSEWHEEL:
    {
        if (configPopupOpen_ && configs_.size() > 5)
        {
            const int maximumScroll = std::max(0, static_cast<int>(configs_.size()) - 5);
            const int wheelSteps = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            configScrollIndex_ = std::clamp(configScrollIndex_ - wheelSteps, 0, maximumScroll);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (selectedPage_ == 3 && !colorPickerOpen_ && configs_.size() > 7)
        {
            POINT point{
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)
            };
            ScreenToClient(hwnd_, &point);
            mouse_ = point;

            const bool overConfigList = Hit(
                static_cast<float>(point.x),
                static_cast<float>(point.y),
                Layout::RightX,
                102.0f,
                Layout::RightRight,
                382.0f);

            if (overConfigList)
            {
                const int maximumScroll = std::max(
                    0,
                    static_cast<int>(configs_.size()) - 7);

                const int wheelSteps =
                    GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

                configScrollIndex_ = std::clamp(
                    configScrollIndex_ - wheelSteps,
                    0,
                    maximumScroll);

                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }
        }

        break;
    }

    case WM_CHAR:
        if (configNameFocused_ && !colorPickerOpen_)
        {
            const wchar_t character = static_cast<wchar_t>(wParam);

            if (character == L'\b')
            {
                if (!configName_.empty())
                    configName_.pop_back();
            }
            else if (character == L'\r' || character == L'\n')
            {
                configNameFocused_ = false;
            }
            else if (character >= 32 && configName_.size() < 32 &&
                (std::iswalnum(character) || character == L' ' ||
                    character == L'-' || character == L'_'))
            {
                configName_.push_back(character);
            }

            InvalidateRect(hwnd_, nullptr, FALSE);
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_RETURN && configNameFocused_)
        {
            configNameFocused_ = false;
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }

        if (wParam == VK_ESCAPE)
        {
            if (colorPickerOpen_ && pickerTarget_)
            {
                *pickerTarget_ = pickerOriginal_;
                colorPickerOpen_ = false;
                pickerTarget_ = nullptr;
                activeColorArea_ = -1;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            else if (openCombo_ != 0)
            {
                openCombo_ = 0;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            else if (configNameFocused_)
            {
                configNameFocused_ = false;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            else if (configPopupOpen_)
            {
                configPopupOpen_ = false;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            else
            {
                PostMessageW(hwnd_, WM_CLOSE, 0, 0);
            }
        }
        return 0;

    case WM_SYSCOMMAND:
        // Keep normal Windows taskbar minimize/restore behavior for this
        // otherwise borderless popup window.
        if ((wParam & 0xFFF0) == SC_RESTORE)
        {
            ShowWindow(hwnd_, SW_RESTORE);
            SetForegroundWindow(hwnd_);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return 0;
        }
        break;

    case WM_SIZE:
        if (renderTarget_)
            renderTarget_->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT paint{};
        BeginPaint(hwnd_, &paint);
        Render();
        EndPaint(hwnd_, &paint);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_SETCURSOR:
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        return TRUE;

    case WM_DESTROY:
        KillTimer(hwnd_, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd_, message, wParam, lParam);
}
