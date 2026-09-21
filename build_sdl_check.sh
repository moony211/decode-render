#! /bin/sh
set -e
g++ -std=c++20 tools/sdl_check.cpp $(pkg-config --cflags --libs sdl2) -o /tmp/sdl_check
/tmp/sdl_check