add_library(ZipLoader ziploader/ziploader.cpp)
add_library(Resource resource/resource.cpp)
target_link_libraries (Resource ${SDL2_LIBRARY})