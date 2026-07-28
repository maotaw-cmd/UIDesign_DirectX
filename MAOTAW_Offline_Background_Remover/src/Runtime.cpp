#include "App.h"

void WriteWorkerScript()
{
    const char* code =
        "import os, sys, traceback\n"
        "from io import BytesIO\n"
        "os.environ.setdefault('OMP_NUM_THREADS','4')\n"
        "\n"
        "def clamp01(v):\n"
        "    try:\n"
        "        v = float(v)\n"
        "    except Exception:\n"
        "        v = 0.55\n"
        "    if v < 0.0:\n"
        "        v = 0.0\n"
        "    if v > 1.0:\n"
        "        v = 1.0\n"
        "    return v\n"
        "\n"
        "def refine_result(original, cutout, feather):\n"
        "    from PIL import Image, ImageFilter, ImageEnhance\n"
        "\n"
        "    original = original.convert('RGBA')\n"
        "    cutout = cutout.convert('RGBA')\n"
        "    r, g, b, alpha = cutout.split()\n"
        "\n"
        "    contrast = 1.24 - feather * 0.34\n"
        "    blur_radius = 0.10 + feather * 1.20\n"
        "    weak_threshold = int(14 + feather * 18)\n"
        "    weak_scale = 0.22 + feather * 0.38\n"
        "\n"
        "    alpha = ImageEnhance.Contrast(alpha).enhance(contrast)\n"
        "    if blur_radius > 0.01:\n"
        "        alpha = alpha.filter(ImageFilter.GaussianBlur(blur_radius))\n"
        "\n"
        "    src = original.load()\n"
        "    mask = alpha.load()\n"
        "    out = Image.new('RGBA', original.size, (0, 0, 0, 0))\n"
        "    dst = out.load()\n"
        "    w, h = original.size\n"
        "\n"
        "    for y in range(h):\n"
        "        for x in range(w):\n"
        "            rr, gg, bb, _ = src[x, y]\n"
        "            a = mask[x, y]\n"
        "\n"
        "            if a <= 6:\n"
        "                dst[x, y] = (0, 0, 0, 0)\n"
        "                continue\n"
        "\n"
        "            if 8 < a < 245 and rr > 185 and gg > 185 and bb > 185:\n"
        "                af = a / 255.0\n"
        "                rr = int(max(0, min(255, (rr - 255 * (1.0 - af)) / max(0.001, af))))\n"
        "                gg = int(max(0, min(255, (gg - 255 * (1.0 - af)) / max(0.001, af))))\n"
        "                bb = int(max(0, min(255, (bb - 255 * (1.0 - af)) / max(0.001, af))))\n"
        "\n"
        "            if a < weak_threshold:\n"
        "                a = int(a * weak_scale)\n"
        "\n"
        "            dst[x, y] = (rr, gg, bb, a)\n"
        "\n"
        "    return out\n"
        "\n"
        "def main():\n"
        "    if len(sys.argv) != 5:\n"
        "        raise RuntimeError('Expected input path, output path, feather value, and model name')\n"
        "\n"
        "    from rembg import remove, new_session\n"
        "    from PIL import Image\n"
        "\n"
        "    feather = clamp01(sys.argv[3])\n"
        "    model_name = sys.argv[4]\n"
        "    original = Image.open(sys.argv[1]).convert('RGBA')\n"
        "    session = new_session(model_name)\n"
        "\n"
        "    erode_size = int(round(6 - feather * 4))\n"
        "    fg_threshold = int(round(246 - feather * 10))\n"
        "    bg_threshold = int(round(8 + (1.0 - feather) * 4))\n"
        "\n"
        "    result = remove(\n"
        "        original,\n"
        "        session=session,\n"
        "        alpha_matting=True,\n"
        "        alpha_matting_foreground_threshold=fg_threshold,\n"
        "        alpha_matting_background_threshold=bg_threshold,\n"
        "        alpha_matting_erode_size=erode_size,\n"
        "        post_process_mask=False\n"
        "    )\n"
        "\n"
        "    refined = refine_result(original, result, feather)\n"
        "    refined.save(sys.argv[2], 'PNG', optimize=True)\n"
        "\n"
        "if __name__ == '__main__':\n"
        "    try:\n"
        "        main()\n"
        "    except Exception:\n"
        "        traceback.print_exc()\n"
        "        raise\n";

    WriteUtf8File(WorkerScript(), code);
}

bool ValidateEngine()
{
    WriteWorkerScript();
    SetEnvironmentVariableW(L"U2NET_HOME", Models().c_str());

    return RunHidden(
        Python(),
        L"-c \"from rembg import remove, new_session; from PIL import Image; print('ENGINE_OK')\"");
}

bool PrepareRuntime()
{
    std::wstring rt = Runtime();

    if (!Exists(Python()))
    {
        g_status = L"Downloading offline AI runtime...";
        g_progressTarget = 0.10f;
        Refresh();

        std::wstring zip = AppFolder() + L"\\python.zip";
        if (!Download(
                L"https://www.python.org/ftp/python/3.11.9/python-3.11.9-embeddable-amd64.zip",
                zip))
        {
            g_status = L"Runtime download failed. Check your internet.";
            return false;
        }

        g_status = L"Extracting runtime...";
        g_progressTarget = 0.25f;
        Refresh();

        std::wstring ps =
            L"-NoProfile -ExecutionPolicy Bypass -Command \"Expand-Archive -LiteralPath " +
            Q(zip) +
            L" -DestinationPath " +
            Q(rt) +
            L" -Force\"";

        if (!RunHidden(L"powershell.exe", ps))
        {
            g_status = L"Could not extract the runtime. See engine.log.";
            return false;
        }

        DeleteFileW(zip.c_str());

        std::wstring pth = rt + L"\\python311._pth";
        std::wifstream in(pth);
        std::wstring out;
        std::wstring line;

        while (std::getline(in, line))
        {
            if (line == L"#import site")
            {
                line = L"import site";
            }
            out += line + L"\n";
        }

        in.close();
        std::wofstream f(pth, std::ios::trunc);
        f << out;
        f.close();
    }

    const std::wstring marker = rt + L"\\rembg_ready_best_v4.txt";
    bool valid = Exists(marker) && ValidateEngine();

    if (!valid)
    {
        DeleteFileW(marker.c_str());

        g_status = L"Repairing background removal engine...";
        g_progressTarget = 0.40f;
        Refresh();

        std::wstring pip = AppFolder() + L"\\get-pip.py";
        if (!Download(L"https://bootstrap.pypa.io/get-pip.py", pip) ||
            !RunHidden(Python(), Q(pip) + L" --disable-pip-version-check"))
        {
            g_status = L"Could not prepare Python packages. See engine.log.";
            return false;
        }

        DeleteFileW(pip.c_str());

        g_status = L"Installing AI libraries. This can take a few minutes...";
        g_progressTarget = 0.78f;
        Refresh();

        if (!RunHidden(
                Python(),
                L"-m pip install --upgrade --force-reinstall --disable-pip-version-check "
                L"--no-warn-script-location \"rembg[cpu]==2.0.77\" pillow"))
        {
            g_status = L"AI installation failed. See engine.log.";
            return false;
        }

        g_status = L"Checking the background removal engine...";
        g_progressTarget = 0.94f;
        Refresh();

        if (!ValidateEngine())
        {
            g_status = L"AI engine setup failed. See engine.log.";
            return false;
        }

        std::wofstream done(marker);
        done << L"ready";
    }

    g_progressTarget = 1.0f;
    g_progress = 1.0f;
    g_ready = true;
    g_status = L"";
    return true;
}
