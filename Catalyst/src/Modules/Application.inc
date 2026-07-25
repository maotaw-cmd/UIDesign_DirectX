


// Win32 input, resize, startup graphics and window-message handling.
// Included once through ProjectAll.h. Do not add this file to Source Files.

// ===== BEGIN App/Application.hpp =====

    // ------------------------------------------------------------------------
    // Window handling
    // ------------------------------------------------------------------------

    HRESULT InitializeGraphics()
    {
        HRESULT hr = CreateFactories();
        if (FAILED(hr))
            return hr;

        hr = CreateD3DDeviceAndSwapChain();
        if (FAILED(hr))
            return hr;

        hr = CreateSizeResources();
        if (FAILED(hr))
            return hr;

        return Create3DResources();
    }

    void Resize(UINT width, UINT height)
    {
        if (!app.swapChain || width == 0 || height == 0)
            return;

        app.width = width;
        app.height = height;
        CreateSizeResources();
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            app.hwnd = hwnd;

            if (FAILED(InitializeGraphics()))
            {
                MessageBoxW(
                    hwnd,
                    L"Could not initialise Direct3D 11 / Direct2D.",
                    L"Initialisation error",
                    MB_ICONERROR);
                return -1;
            }

            RefreshConfigs();
            SetTimer(hwnd, 1, 16, nullptr);
            return 0;

        case WM_NCCALCSIZE:
            if (wParam)
                return 0;
            break;

        case WM_SIZE:
            Resize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_TIMER:
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT paint{};
            BeginPaint(hwnd, &paint);
            Render();
            EndPaint(hwnd, &paint);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            app.mouse.x = GET_X_LPARAM(lParam);
            app.mouse.y = GET_Y_LPARAM(lParam);

            if (app.draggingVisualPopup && app.visualPopup != VisualPopup::None)
            {
                app.visualPopupX = static_cast<float>(app.mouse.x) - app.visualPopupDragOffsetX;
                app.visualPopupY = static_cast<float>(app.mouse.y) - app.visualPopupDragOffsetY;
                InvalidateRect(hwnd, nullptr, FALSE);
            }

            if (app.draggingConfigPanel && app.configOpen)
            {
                app.configPanelX = static_cast<float>(app.mouse.x) - app.configPanelDragOffsetX;
                app.configPanelY = static_cast<float>(app.mouse.y) - app.configPanelDragOffsetY;
                InvalidateRect(hwnd, nullptr, FALSE);
            }

            if (app.draggingModel)
            {
                const float currentX = static_cast<float>(app.mouse.x);
                app.modelYaw += (currentX - app.previousMouseX) * 0.012f;
                app.previousMouseX = currentX;
            }

            if (app.mouseDown && app.activeSlider != SliderId::None)
                InvalidateRect(hwnd, nullptr, FALSE);

            TRACKMOUSEEVENT trackMouse{};
            trackMouse.cbSize = sizeof(trackMouse);
            trackMouse.dwFlags = TME_LEAVE;
            trackMouse.hwndTrack = hwnd;
            trackMouse.dwHoverTime = 0;
            TrackMouseEvent(&trackMouse);
            UpdatePointerCursor();

            return 0;
        }

        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT)
            {
                UpdatePointerCursor();
                return TRUE;
            }
            break;

        case WM_MOUSELEAVE:
            app.mouse = POINT{ -1000, -1000 };
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
        {
            SetCapture(hwnd);
            app.mouseDown = true;
            app.clicked = true;
            app.clickConsumed = false;
            app.mouse.x = GET_X_LPARAM(lParam);
            app.mouse.y = GET_Y_LPARAM(lParam);

            if (!app.colourPickerOpen &&
                Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), CloseButtonRect()))
            {
                DestroyWindow(hwnd);
                return 0;
            }
            else if (!app.colourPickerOpen &&
                Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), SaveButtonRect()))
            {
                app.configOpen = !app.configOpen;
                app.settingsOpen = false;
                app.visualPopup = VisualPopup::None;
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
                app.configNameFocused = false;
                if (app.configOpen)
                    RefreshConfigs();
                app.clickConsumed = true;
            }
            else if (!app.colourPickerOpen &&
                Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), GearButtonRect()))
            {
                app.visualPopup = VisualPopup::None;
                app.visualPopupRect = Rect(0, 0, 0, 0);
                app.visualPopupPositioned = false;
                app.draggingVisualPopup = false;
                app.healthStyleDropdownOpen = false;
                app.popupDropdown = PopupDropdown::None;
                app.settingsOpen = !app.settingsOpen;
                app.configOpen = false;
                app.configNameFocused = false;
                app.clickConsumed = true;
            }
            else if (app.configOpen)
            {
                // Drag the configuration popup from its title/header area.
                const float mx = static_cast<float>(app.mouse.x);
                const float my = static_cast<float>(app.mouse.y);
                const D2D1_RECT_F configHeader = Rect(
                    app.configPanelRect.left,
                    app.configPanelRect.top,
                    app.configPanelRect.right - 48.0f,
                    app.configPanelRect.top + 46.0f);

                if (Hit(mx, my, configHeader))
                {
                    app.draggingConfigPanel = true;
                    app.configPanelDragOffsetX = mx - app.configPanelRect.left;
                    app.configPanelDragOffsetY = my - app.configPanelRect.top;
                    app.configNameFocused = false;
                    app.clickConsumed = true;
                }
                // Other configuration controls are handled during rendering.
            }
            else
            {
                const float mx = static_cast<float>(app.mouse.x);
                const float my = static_cast<float>(app.mouse.y);
                const bool insideVisualPopup =
                    app.visualPopup != VisualPopup::None &&
                    Hit(mx, my, app.visualPopupRect);
                const bool insidePopupDropdown =
                    app.healthStyleDropdownOpen &&
                    Hit(mx, my, app.healthStylePopupRect);

                if (!app.colourPickerOpen &&
                    app.visualPopup != VisualPopup::None &&
                    insideVisualPopup)
                {
                    const D2D1_RECT_F popupHeader = Rect(
                        app.visualPopupRect.left,
                        app.visualPopupRect.top,
                        app.visualPopupRect.right - 42.0f,
                        app.visualPopupRect.top + 39.0f);

                    if (Hit(mx, my, popupHeader))
                    {
                        app.draggingVisualPopup = true;
                        app.visualPopupDragOffsetX = mx - app.visualPopupRect.left;
                        app.visualPopupDragOffsetY = my - app.visualPopupRect.top;
                        app.activeSlider = SliderId::None;
                        app.clickConsumed = true;
                    }
                }

                // Let the visual-row gear buttons handle popup toggling themselves.
                // Previously this block closed the popup before the gear received the
                // click, so clicking the active gear immediately reopened it.
                const bool insideVisualControls = Hit(mx, my, app.visualScrollRect);
                if (!app.colourPickerOpen &&
                    !app.healthStyleDropdownOpen &&
                    app.visualPopup != VisualPopup::None &&
                    !insideVisualPopup &&
                    !insidePopupDropdown &&
                    !insideVisualControls)
                {
                    app.visualPopup = VisualPopup::None;
                    app.visualPopupRect = Rect(0, 0, 0, 0);
                    app.visualPopupPositioned = false;
                    app.draggingVisualPopup = false;
                }

                if (!app.colourPickerOpen &&
                    !app.healthStyleDropdownOpen &&
                    !insideVisualPopup &&
                    Hit(mx, my, app.previewRect))
                {
                    app.draggingModel = true;
                    app.previousMouseX = mx;
                    app.clickConsumed = true;
                }
            }

            if (!app.colourPickerOpen && app.mouse.y <= static_cast<LONG>(TOP_H) && !app.clickConsumed)
            {
                ReleaseCapture();
                app.mouseDown = false;
                SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }

            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONUP:
            app.mouseDown = false;
            app.activeSlider = SliderId::None;
            app.activePickerArea = -1;
            app.draggingModel = false;
            app.draggingVisualPopup = false;
            app.draggingConfigPanel = false;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_CHAR:
            if (app.configOpen && app.configNameFocused)
            {
                const wchar_t character = static_cast<wchar_t>(wParam);
                if (character == L'\b')
                {
                    if (!app.configName.empty())
                        app.configName.pop_back();
                }
                else if (character >= 32 && app.configName.size() < 36 &&
                    (std::iswalnum(character) || character == L' ' ||
                        character == L'-' || character == L'_'))
                {
                    app.configName.push_back(character);
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;

        case WM_MOUSEWHEEL:
            if (app.configOpen)
            {
                const int direction = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -1 : 1;
                const float maxScroll = (std::max)(0.0f, static_cast<float>(app.configs.size()) - 7.0f);
                app.configScroll = Clamp(app.configScroll + static_cast<float>(direction), 0.0f, maxScroll);
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }

            if (!app.settingsOpen && !app.colourPickerOpen &&
                Hit(static_cast<float>(app.mouse.x), static_cast<float>(app.mouse.y), app.visualScrollRect))
            {
                const float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
                app.visualScrollTarget = Clamp(app.visualScrollTarget - delta * 72.0f,
                    0.0f, app.visualScrollMax);
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            break;

        case WM_KEYDOWN:
            // Capture a key (optionally Ctrl + key) for the currently open visual popup.
            if (app.capturingVisualHotkey && app.hotkeyCapturePopup != VisualPopup::None)
            {
                const int vk = static_cast<int>(wParam);
                const int index = static_cast<int>(app.hotkeyCapturePopup);

                if (vk == VK_ESCAPE)
                {
                    app.capturingVisualHotkey = false;
                    app.hotkeyCapturePopup = VisualPopup::None;
                }
                else if (vk == VK_BACK || vk == VK_DELETE)
                {
                    app.visualHotkeyVk[index] = 0;
                    app.visualHotkeyCtrl[index] = false;
                    app.capturingVisualHotkey = false;
                    app.hotkeyCapturePopup = VisualPopup::None;
                }
                else if (vk != VK_CONTROL && vk != VK_LCONTROL && vk != VK_RCONTROL &&
                    vk != VK_SHIFT && vk != VK_LSHIFT && vk != VK_RSHIFT &&
                    vk != VK_MENU && vk != VK_LMENU && vk != VK_RMENU)
                {
                    app.visualHotkeyVk[index] = vk;
                    app.visualHotkeyCtrl[index] = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                    app.capturingVisualHotkey = false;
                    app.hotkeyCapturePopup = VisualPopup::None;
                }

                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }

            // Apply configured hotkeys. Ignore auto-repeat so one press toggles once.
            if ((lParam & (1LL << 30)) == 0)
            {
                const bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                auto hotkeyPressed = [&](VisualPopup popup)
                {
                    const int index = static_cast<int>(popup);
                    return app.visualHotkeyVk[index] != 0 &&
                        app.visualHotkeyVk[index] == static_cast<int>(wParam) &&
                        app.visualHotkeyCtrl[index] == ctrlDown;
                };

                if (hotkeyPressed(VisualPopup::Box)) app.settings.box = !app.settings.box;
                else if (hotkeyPressed(VisualPopup::HealthBar)) app.settings.healthBar = !app.settings.healthBar;
                else if (hotkeyPressed(VisualPopup::Snapline)) app.settings.snapline = !app.settings.snapline;
                else if (hotkeyPressed(VisualPopup::Skeleton)) app.settings.skeleton = !app.settings.skeleton;
                else if (hotkeyPressed(VisualPopup::SoundWalk)) app.settings.soundWalk = !app.settings.soundWalk;
                else if (hotkeyPressed(VisualPopup::ModelOutline)) app.settings.visualGlow = !app.settings.visualGlow;
            }

            if (app.configOpen && app.configNameFocused && wParam == VK_RETURN)
            {
                app.configNameFocused = false;
                SaveNamedConfig();
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }

            if (wParam == VK_ESCAPE)
            {
                if (app.configNameFocused)
                {
                    app.configNameFocused = false;
                }
                else if (app.configOpen)
                {
                    app.configOpen = false;
                }
                else if (app.colourPickerOpen)
                {
                    if (app.colourTarget)
                        *app.colourTarget = app.originalColour;

                    app.colourPickerOpen = false;
                    app.colourTarget = nullptr;
                    app.activePickerArea = -1;
                }
                else if (app.healthStyleDropdownOpen)
                {
                    app.healthStyleDropdownOpen = false;
                    app.popupDropdown = PopupDropdown::None;
                }
                else if (app.visualPopup != VisualPopup::None)
                {
                    app.visualPopup = VisualPopup::None;
                    app.visualPopupRect = Rect(0, 0, 0, 0);
                    app.visualPopupPositioned = false;
                    app.draggingVisualPopup = false;
                }
                else
                {
                    PostMessageW(hwnd, WM_CLOSE, 0, 0);
                }

                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
