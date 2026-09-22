cmake_minimum_required(VERSION 3.24)
project(mkvplayer CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 CONFIG REQUIRED)          # SDL2::SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat libavcodec libavutil libswscale libswresample)

add_executable(sdl_check tools/sdl_check.cpp)
target_link_libraries(sdl_check PRIVATE SDL2::SDL2)

add_executable(hello_coro tools/hello_coro.cpp)   # asio는 header-only, 링크 불필요
