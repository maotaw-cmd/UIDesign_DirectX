cmake_minimum_required(VERSION 3.20)
project(Visuals3D LANGUAGES CXX)

add_executable(Visuals3D WIN32
    src/main.cpp
    src/Application.cpp
    src/Config.cpp
)

target_compile_features(Visuals3D PRIVATE cxx_std_17)
target_compile_definitions(Visuals3D PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_include_directories(Visuals3D PRIVATE src)
target_link_libraries(Visuals3D PRIVATE d3d11 dxgi d3dcompiler d2d1 dwrite)
