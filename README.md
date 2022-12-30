NonoGIF: Render Nonogram solutions to GIFs
==========================================

This command line program renders [Nonogram](https://en.wikipedia.org/wiki/Nonogram)
puzzles to GIF files. It provides a range of options to control the look of the
output animation.

Nonograms (also known as Paint by Numbers, Picross, and Pixel Puzzles, amongst
others) are a form of puzzle where the object is to use clues to fill in the
cells in a grid to reveal a picture. The clues are provided as a series of
numbers for each row and column, indicating the length of groups of contiguous
cells along that line that may be filled in.

## Why

This is not a clever or efficient solver. It simply tries every possible
option for every line until the puzzle is complete.

I made it because I was given a Nonogram in a Christmas card and I thought it
would be fun to write a program to solve it, and send my friend a GIF of the
solution.

## Features

* Basic Nonogram solver.
* Reads YAML format input file.
* Command line interface.
* Configurable GIF output.
  - Options to animate line by line, pass by pass or simply render the final
    result to a non-animated GIF.
  - Options to control animation behaviour, e.g. inter frame delays.
  - Options to control appearance, e.g. grid size and border thickness.

## Dependencies

* [libcgif](https://github.com/dloebl/cgif): GIF encoding.
* [libcyaml](https://github.com/tlsa/libcyaml): YAML loading.

## Building

Simply run

```bash
make
```

By default a `release` build is built. To build a `debug` or `sanitizer`
variant, set `VARIANT` accordingly:

```bash
make VARIENT=debug
make VARIANT=sanitizer
```
