# Plat2D
Casual 2D engine for platformer 'Megaman'-like

** early stages **

Not much at the moment, only a very small [Tiled](https://www.mapeditor.org)
parser, to toy with the format (see Test0)

Depends on:
- [SDL 2.0](https://www.libsdl.org/)
- [SDL\_image 2.0](https://www.libsdl.org/projects/SDL_image/)
- [zlib](https://www.zlib.net/)
- [nlohmann](https://github.com/nlohmann/json) JSON parser.
- C++
- (maybe lua for scripting in the future)
- [CMake](https://cmake.org/)

# Building information
```shell
$ cd /path/to/Plat2D
$ mkdir build
# for Windows ...
$ cmake .. -G "MinGW Makefiles"
# or for Linux
$ cmake .. -G "Unix Makefiles"
$ make
$ make install
$ cd ../bin
$ # Enjoy !
```

If you want to build with debug symbol, use instead
```shell
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
```

If you want to build a release, use instead
```shell
$ cmake .. -DCMAKE_BUILD_TYPE=Release
```

# Author
- Vinz
