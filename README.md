# Plat2D
Casual 2D engine for platformer 'Megaman'-like

** early stages **

Not much at the moment, only a very small [Tiled](https://www.mapeditor.org)
parser, to toy with the format (see Test1)

Depends on:
- C++
- SDL 2.0
- SDL\_image 2.0
- Zlib
- [nlohmann](https://github.com/nlohmann/json) JSON parser.
- (maybe lua for scripting in the future)

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
