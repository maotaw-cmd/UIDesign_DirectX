#include "App.h"

void Paint()
{
    if (FAILED(Graphics()))
    {
        return;
    }

    RECT rc = {};
    GetClientRect(g_hwnd, &rc);

    float w = static_cast<float>(rc.right);
    float h = static_cast<float>(rc.bottom);

    g_rt->BeginDraw();
    g_rt->FillRectangle({ 0, 0, w, h }, g_gradient);

    Logo();

    D2D1_RECT_F st = SettingsButtonRect(w);
    if (g_showSettings)
    {
        Brush(255, 255, 255, 0.10f);
        D2D1_ROUNDED_RECT rr = { st, 5, 5 };
        g_rt->FillRoundedRectangle(&rr, g_brush);
    }

    Brush(242, 244, 246);
    SettingsIcon((st.left + st.right) * 0.5f, (st.top + st.bottom) * 0.5f);

    Text(L"Offline Background Remover", { 72, 16, w - 72, 58 }, 244, 246, 248);

    if (!g_ready)
    {
        D2D1_RECT_F p = { 145, 146, w - 145, 392 };
        D2D1_ROUNDED_RECT prc = { p, 12, 12 };

        Brush(14, 18, 22, 0.92f);
        g_rt->FillRoundedRectangle(&prc, g_brush);

        Brush(255, 255, 255, 0.14f);
        g_rt->DrawRoundedRectangle(&prc, g_brush, 1.0f);

        D2D1_ELLIPSE halo = { { (p.left + p.right) * 0.5f, p.top + 58 }, 30.0f, 30.0f };
        Brush(255, 255, 255, 0.05f);
        g_rt->FillEllipse(&halo, g_brush);

        Brush(239, 243, 246, 0.95f);
        ImageIcon((p.left + p.right) * 0.5f, p.top + 58);

        Text(
            L"Preparing the app",
            { p.left + 20, p.top + 92, p.right - 20, p.top + 126 },
            242,
            245,
            248,
            1.0f,
            true);

        Text(
            g_status.c_str(),
            { p.left + 34, p.top + 128, p.right - 34, p.top + 168 },
            150,
            158,
            166,
            0.96f,
            true);

        D2D1_RECT_F tr = { p.left + 54, p.top + 198, p.right - 54, p.top + 202 };
        D2D1_ROUNDED_RECT trr = { tr, 2, 2 };

        Brush(26, 31, 36, 1.0f);
        g_rt->FillRoundedRectangle(&trr, g_brush);

        D2D1_RECT_F fi = tr;
        fi.right = tr.left + (tr.right - tr.left) * g_progress.load();
        D2D1_ROUNDED_RECT fir = { fi, 2, 2 };

        Brush(244, 246, 248, 1.0f);
        g_rt->FillRoundedRectangle(&fir, g_brush);

        std::wstring pr = std::to_wstring(static_cast<int>(std::lround(g_progress.load() * 100.0f))) + L"%";
        Text(pr.c_str(), { p.left, p.top + 210, p.right, p.top + 238 }, 175, 182, 189, 0.92f, true);
    }
    else if (!g_inputBitmap)
    {
        D2D1_RECT_F d = ChooseCardRect(w);
        bool hv = Inside(g_mouseX, g_mouseY, d);
        D2D1_ROUNDED_RECT rr = { d, 12, 12 };

        Brush(hv ? 17 : 15, hv ? 21 : 19, hv ? 26 : 24, 0.96f);
        g_rt->FillRoundedRectangle(&rr, g_brush);

        Brush(255, 255, 255, hv ? 0.14f : 0.09f);
        g_rt->DrawRoundedRectangle(&rr, g_brush, 1.0f);

        D2D1_ELLIPSE halo = { { (d.left + d.right) * 0.5f, d.top + 82 }, 30.0f, 30.0f };
        Brush(255, 255, 255, hv ? 0.07f : 0.05f);
        g_rt->FillEllipse(&halo, g_brush);

        Brush(240, 243, 246, 0.98f);
        ImageIcon((d.left + d.right) * 0.5f, d.top + 82);

        Text(
            L"Choose an image",
            { d.left + 20, d.top + 124, d.right - 20, d.top + 160 },
            244,
            246,
            248,
            1.0f,
            true);

        Text(
            L"Browse PNG, JPG, JPEG, WebP or BMP",
            { d.left + 30, d.top + 160, d.right - 30, d.top + 190 },
            152,
            160,
            168,
            0.94f,
            true);

        D2D1_RECT_F b = ChooseSelectButtonRect(w);
        D2D1_ROUNDED_RECT br = { b, 7, 7 };
        bool hb = Inside(g_mouseX, g_mouseY, b);

        Brush(hb ? 248 : 239, hb ? 250 : 243, hb ? 252 : 246, 1.0f);
        g_rt->FillRoundedRectangle(&br, g_brush);
        Text(L"Select image", b, 20, 24, 29, 1.0f, true);
    }
    else
    {
        D2D1_RECT_F card = PreviewCardRect(w);
        const wchar_t* title = g_outputBitmap ? L"Background removed" : L"Original";

        Text(title, { card.left, card.top, card.right, card.top + 34 }, 205, 210, 215, 1.0f, true);

        if (g_outputBitmap)
        {
            DrawBitmapFit(g_outputBitmap, { card.left, card.top + 34, card.right, card.bottom }, g_resultAnim);
        }
        else
        {
            DrawBitmapFit(g_inputBitmap, { card.left, card.top + 34, card.right, card.bottom });
        }

        D2D1_RECT_F b = MainButton(w);
        D2D1_ROUNDED_RECT br = { b, 5, 5 };
        bool hb = Inside(g_mouseX, g_mouseY, b);

        Brush(
            g_busy ? 86 : (hb ? 255 : 236),
            g_busy ? 92 : (hb ? 255 : 239),
            g_busy ? 98 : (hb ? 255 : 242));
        g_rt->FillRoundedRectangle(&br, g_brush);

        std::wstring actionText =
            g_busy
                ? (L"Processing " + std::to_wstring(g_processPercent.load()) + L"%")
                : L"Remove background";
        Text(actionText.c_str(), b, 24, 28, 33, 1.0f, true);

        D2D1_RECT_F ch = ChangeButton();
        D2D1_ROUNDED_RECT chr = { ch, 5, 5 };
        bool hc = Inside(g_mouseX, g_mouseY, ch);

        Brush(hc ? 37 : 31, hc ? 42 : 36, hc ? 47 : 41, 0.95f);
        g_rt->FillRoundedRectangle(&chr, g_brush);

        Brush(100, 107, 114, 0.8f);
        g_rt->DrawRoundedRectangle(&chr, g_brush, 1.0f);

        Text(L"Choose another", { ch.left + 8, ch.top, ch.right - 30, ch.bottom }, 225, 229, 233, 1.0f, true);

        Brush(225, 229, 233, 1.0f);
        ImageIcon(ch.right - 18, (ch.top + ch.bottom) * 0.5f);

        if (g_outputBitmap)
        {
            D2D1_RECT_F sv = SaveButton(w);
            D2D1_ROUNDED_RECT sr = { sv, 5, 5 };
            bool hs = Inside(g_mouseX, g_mouseY, sv);

            Brush(hs ? 255 : 236, hs ? 255 : 239, hs ? 255 : 242, 1.0f);
            g_rt->FillRoundedRectangle(&sr, g_brush);

            Brush(24, 28, 33, 1.0f);
            SaveIcon(sv.left + 20, (sv.top + sv.bottom) * 0.5f);
            Text(L"Download PNG", { sv.left + 34, sv.top, sv.right - 6, sv.bottom }, 24, 28, 33, 1.0f, true);
        }
    }

    if (g_showSettings)
    {
        D2D1_RECT_F p = SettingsPopupRect(w);
        D2D1_ROUNDED_RECT rr = { p, 7, 7 };

        Brush(12, 15, 19, 0.96f);
        g_rt->FillRoundedRectangle(&rr, g_brush);

        Brush(82, 89, 96, 0.75f);
        g_rt->DrawRoundedRectangle(&rr, g_brush, 1.0f);

        Text(L"Feather", { p.left + 16, p.top + 10, p.right - 16, p.top + 32 }, 235, 238, 241, 1.0f, false);
        Text(
            L"Affects the next removal result",
            { p.left + 16, p.top + 30, p.right - 16, p.top + 52 },
            135,
            142,
            149,
            0.92f,
            false);

        D2D1_RECT_F tr = FeatherTrackRect(w);
        Brush(51, 57, 64, 1.0f);
        g_rt->FillRectangle(&tr, g_brush);

        D2D1_RECT_F fill = tr;
        fill.right = tr.left + (tr.right - tr.left) * g_featherValue;
        Brush(242, 244, 246, 1.0f);
        g_rt->FillRectangle(&fill, g_brush);

        float knobX = fill.right;
        D2D1_ELLIPSE knob = { { knobX, (tr.top + tr.bottom) * 0.5f }, 6.0f, 6.0f };
        Brush(250, 252, 255, 1.0f);
        g_rt->FillEllipse(&knob, g_brush);

        std::wstring val = L"Value: " + std::to_wstring(static_cast<int>(std::lround(g_featherValue * 100.0f))) + L"%";
        Text(val.c_str(), { p.left + 16, p.top + 100, p.right - 16, p.top + 122 }, 160, 167, 174, 0.95f, false);
    }

    if (!g_status.empty())
    {
        Text(g_status.c_str(), { 210, 518, w - 210, 554 }, 137, 144, 151, 0.9f, true);
    }

    Brush(255, 255, 255, 0.28f);
    D2D1_RECT_F wr = { 1.5f, 1.5f, w - 1.5f, h - 1.5f };
    g_rt->DrawRectangle(&wr, g_brush, 1.0f);

    HRESULT hr = g_rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        Release(&g_gradient);
        Release(&g_stops);
        Release(&g_brush);
        Release(&g_rt);
    }
}

bool ChooseImage()
{
    IFileOpenDialog* d = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d))))
    {
        return false;
    }

    COMDLG_FILTERSPEC f[] = {
        { L"Images", L"*.png;*.jpg;*.jpeg;*.webp;*.bmp" }
    };

    d->SetFileTypes(1, f);
    d->SetTitle(L"Choose an image");

    HRESULT hr = d->Show(g_hwnd);
    if (SUCCEEDED(hr))
    {
        IShellItem* i = nullptr;
        if (SUCCEEDED(d->GetResult(&i)))
        {
            PWSTR p = nullptr;
            if (SUCCEEDED(i->GetDisplayName(SIGDN_FILESYSPATH, &p)))
            {
                g_inputPath = p;

                Release(&g_inputBitmap);
                Release(&g_outputBitmap);

                g_outputPath.clear();
                g_outputPixels.clear();
                g_outputPixelWidth = 0;
                g_outputPixelHeight = 0;
                g_outputStride = 0;
                g_resultAnim = 0;
                g_eraserMode = false;

                if (SUCCEEDED(LoadBitmap(g_inputPath, &g_inputBitmap)))
                {
                    g_status = L"";
                }
                else
                {
                    g_status = L"Could not open this image.";
                }

                CoTaskMemFree(p);
            }
            i->Release();
        }
    }

    d->Release();
    InvalidateRect(g_hwnd, nullptr, FALSE);
    return SUCCEEDED(hr);
}

void Process()
{
    if (g_busy || !g_ready || g_inputPath.empty())
    {
        return;
    }

    g_busy = true;
    g_processPercent = 1;
    g_status = L"Removing background locally...";
    Refresh();

    std::thread([]
    {
        std::thread progressThread([]
        {
            int ticksAt94 = 0;

            while (g_busy)
            {
                int p = g_processPercent.load();

                if (p < 94)
                {
                    int step = p < 35 ? 3 : (p < 70 ? 2 : 1);
                    g_processPercent = (std::min)(94, p + step);
                }
                else
                {
                    ++ticksAt94;

                    if (ticksAt94 == 2)
                    {
                        g_status = L"Loading the AI model. First use can take several minutes...";
                    }

                    if (ticksAt94 > 10 && p < 99)
                    {
                        g_processPercent = (std::min)(99, p + 1);
                    }
                }

                Refresh();
                Sleep(500);
            }
        });

        std::wstring out = AppFolder() + L"\\result.png";
        DeleteFileW(out.c_str());

        SetEnvironmentVariableW(L"U2NET_HOME", Models().c_str());
        WriteWorkerScript();

        wchar_t featherBuf[32];
        swprintf_s(featherBuf, L"%.3f", g_featherValue);

        std::wstring baseArgs =
            Q(WorkerScript()) +
            L" " +
            Q(g_inputPath) +
            L" " +
            Q(out) +
            L" " +
            featherBuf +
            L" ";

        bool ok = RunHiddenLogged(
                      Python(),
                      baseArgs + L"birefnet-general",
                      LogPath(),
                      8 * 60 * 1000) &&
                  Exists(out);

        if (!ok)
        {
            DeleteFileW(out.c_str());
            g_status = L"Best-quality model was too slow. Finishing with the fast offline model...";
            g_processPercent = 96;
            Refresh();

            ok = RunHiddenLogged(
                     Python(),
                     baseArgs + L"u2net",
                     LogPath(),
                     5 * 60 * 1000) &&
                 Exists(out);
        }

        g_processPercent = ok ? 100 : 0;

        if (ok)
        {
            g_outputPath = out;
            g_status = L"";
        }
        else
        {
            g_status = L"Processing failed or timed out — see engine.log.";
        }

        g_busy = false;

        if (progressThread.joinable())
        {
            progressThread.join();
        }

        PostMessageW(g_hwnd, WM_APP + 2, ok ? 1 : 0, 0);
    }).detach();
}

void Save()
{
    if (g_outputPixels.empty() && !Exists(g_outputPath))
    {
        return;
    }

    IFileSaveDialog* d = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d))))
    {
        return;
    }

    COMDLG_FILTERSPEC f[] = {
        { L"PNG image", L"*.png" }
    };

    d->SetFileTypes(1, f);
    d->SetDefaultExtension(L"png");
    d->SetFileName(L"background_removed.png");

    if (SUCCEEDED(d->Show(g_hwnd)))
    {
        IShellItem* i = nullptr;
        if (SUCCEEDED(d->GetResult(&i)))
        {
            PWSTR p = nullptr;
            if (SUCCEEDED(i->GetDisplayName(SIGDN_FILESYSPATH, &p)))
            {
                bool ok =
                    !g_outputPixels.empty()
                        ? SaveEditedOutputPng(p)
                        : (CopyFileW(g_outputPath.c_str(), p, FALSE) != 0);

                g_status = ok ? L"PNG saved successfully." : L"Could not save the PNG.";
                CoTaskMemFree(p);
            }
            i->Release();
        }
    }

    d->Release();
    InvalidateRect(g_hwnd, nullptr, FALSE);
}
