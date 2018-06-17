project(TestPngZip)
add_executable(TestPngZip src/main.cpp)
target_link_libraries(TestPngZip Resource ZipLoader ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY} ${ZLIB_LIBRARY})

install(TARGETS TestPngZip RUNTIME DESTINATION ${BIN_DIR})