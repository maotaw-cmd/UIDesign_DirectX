#include "App.h"

D2D1_COLOR_F C(BYTE r, BYTE g, BYTE b, float a)
{
    return { r / 255.0f, g / 255.0f, b / 255.0f, a };
}

void Brush(BYTE r, BYTE g, BYTE b, float a)
{
    D2D1_COLOR_F c = C(r, g, b, a);
    if (g_brush)
    {
        g_brush->SetColor(&c);
    }
}

bool Inside(float x, float y, const D2D1_RECT_F& r)
{
    return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
}

std::wstring Q(const std::wstring& s)
{
    return L"\"" + s + L"\"";
}

bool Exists(const std::wstring& p)
{
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring AppFolder()
{
    wchar_t p[MAX_PATH] = {};

    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, p)))
    {
        return L".\\MAOTAW_BackgroundRemover";
    }

    std::wstring f = std::wstring(p) + L"\\MAOTAW\\BackgroundRemover";
    SHCreateDirectoryExW(nullptr, f.c_str(), nullptr);
    return f;
}

std::wstring Runtime()
{
    std::wstring p = AppFolder() + L"\\runtime";
    SHCreateDirectoryExW(nullptr, p.c_str(), nullptr);
    return p;
}

std::wstring Python()
{
    return Runtime() + L"\\python.exe";
}

std::wstring Models()
{
    std::wstring p = AppFolder() + L"\\models";
    SHCreateDirectoryExW(nullptr, p.c_str(), nullptr);
    return p;
}

void Refresh()
{
    if (g_hwnd)
    {
        PostMessageW(g_hwnd, WM_APP + 1, 0, 0);
    }
}

std::wstring LogPath()
{
    return AppFolder() + L"\\engine.log";
}

std::wstring WorkerScript()
{
    return AppFolder() + L"\\remove_bg.py";
}

bool WriteUtf8File(const std::wstring& path, const std::string& text)
{
    HANDLE f = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (f == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD written = 0;
    BOOL ok = WriteFile(f, text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
    CloseHandle(f);

    return ok && written == text.size();
}

bool RunHiddenLogged(
    const std::wstring& exe,
    const std::wstring& args,
    const std::wstring& logPath,
    DWORD timeoutMs)
{
    std::wstring cmd = Q(exe) + L" " + args;
    std::vector<wchar_t> buffer(cmd.begin(), cmd.end());
    buffer.push_back(0);

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    HANDLE log = CreateFileW(
        logPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (log == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = log;
    si.hStdError = log;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi = {};

    BOOL created = CreateProcessW(
        nullptr,
        buffer.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        Runtime().c_str(),
        &si,
        &pi);

    if (!created)
    {
        CloseHandle(log);
        return false;
    }

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    bool ok = false;

    if (waitResult == WAIT_TIMEOUT)
    {
        TerminateProcess(pi.hProcess, 1);
        std::ofstream out(logPath, std::ios::app | std::ios::binary);
        out << "\r\n[ghostwriter] process timed out after " << timeoutMs << " ms\r\n";
        out.close();
    }
    else
    {
        DWORD code = 1;
        GetExitCodeProcess(pi.hProcess, &code);
        ok = (code == 0);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(log);
    return ok;
}

bool RunHidden(const std::wstring& exe, const std::wstring& args)
{
    return RunHiddenLogged(exe, args, LogPath());
}

bool Download(const std::wstring& url, const std::wstring& path)
{
    DeleteFileW(path.c_str());
    return SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), path.c_str(), 0, nullptr));
}
