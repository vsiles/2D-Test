project(Test3)
add_executable(Test3 src/main.cpp src/resource.cpp)
target_link_libraries(Test3 ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})

install(TARGETS Test3 RUNTIME DESTINATION ${BIN_DIR})
