@echo off
setlocal
where cmake >nul 2>nul || (
  echo CMake was not found. Install Visual Studio 2022 with Desktop development with C++ and CMake tools.
  pause
  exit /b 1
)
cmake -S . -B build -A x64
if errorlevel 1 goto :fail
cmake --build build --config Release
if errorlevel 1 goto :fail
echo.
echo Build complete: build\Release\Visuals3D.exe
pause
exit /b 0
:fail
echo.
echo Build failed. Review the compiler output above.
pause
exit /b 1
