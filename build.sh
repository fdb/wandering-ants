#!/bin/sh
clang -g -o ants -Wall -Werror `pkg-config --cflags --libs sdl2` -framework OpenGL vendor/nanovg/nanovg.c src/ants.c
