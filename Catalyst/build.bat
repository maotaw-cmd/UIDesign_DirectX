@echo off
setlocal
where msbuild >nul 2>nul
if errorlevel 1 (
    echo MSBuild was not found.
    echo Open an x64 Native Tools Command Prompt for Visual Studio and run this file again.
    pause
    exit /b 1
)
msbuild Catalyst.sln /m /p:Configuration=Release /p:Platform=x64
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)
echo.
echo Build complete: x64\Release\Catalyst.exe
pause
