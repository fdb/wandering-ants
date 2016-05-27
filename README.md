# Wandering Ants
Simple algorithm for crawling and reproducing ants, initially designed for a [NodeBox](https://www.nodebox.net/) workshop. Each time two ants meet, they produce offspring. However, nobody dies, so the population grows explosively.

![Screenshot](screenshot.png)

[Video](https://youtu.be/jYs9WNQfG80)

## Building (OS X, with Homebrew)

  brew install sdl2
  ./build.sh

## Running

  ./ants

Note that, because none of the ants die, the algorithm will keep on generating new ants, grinding the algorithm to a halt. That's sort of the point.

## Hacking
The `src/ants.c` file has a number of `#define`s you can tweak to your hearts content. Try changing the RANDOM_SEED.

To save out frames, set `SAVE_FRAME` to `1`. Frames will be saved out in TGA format in the `OUT_DIRECTORY`.
