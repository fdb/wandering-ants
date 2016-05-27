#!/bin/sh
clang -O3 -g -o ants -Ivendor/nanovg `pkg-config --cflags --libs sdl2` -framework OpenGL src/ants.c vendor/nanovg/nanovg.c

