# Solver4

Algorithm for finding a solution for given Rubik's cube scramble.

At the moment the solver is working, it is pretty fast actually (faster than
I tought it would be and only tested on a few tests written by hand :P).

## Building
Simply run `make` in the root directory of the project. I compile it on Linux.

## TODOs:
  - Move code from [cube.h](./src/cube.h) to a normal source file, not a header file...
  - Clean up the code in general
  - Document the code (mainly [cube.h](./src/cube.h)), in particular describe
  how a cube state is represented and how a single rotation is performed
  (I think its done pretty cool actually)
  - Add a config file / command line options (for example it would be nice to
  tweak heuristic parameters)
  - Do some benchmarks! Explore some other heuristic functions, check for
  bottlenecks etc.
  - Maybe make a library?
