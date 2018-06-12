# Plat2D
Casual 2D engine for platformer 'Megaman'-like

** early stages **

Not much at the moment, only a very small [Tiled](https://www.mapeditor.org)
parser, to toy with the format (see Test1)

Built using C++ / SDL 2.0 (maybe lua for scripting in the future) and using
[nlohmann](https://github.com/nlohmann/json) JSON parser.

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

# Author
- Vinz
