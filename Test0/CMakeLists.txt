project(Test0)
add_executable(Test0 src/main.cpp src/base64.cpp)
target_link_libraries(Test0 ${ZLIB_LIBRARY} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})

install(TARGETS Test0 RUNTIME DESTINATION ${BIN_DIR})
