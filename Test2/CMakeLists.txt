project(Test2)
add_executable(Test2 src/main.cpp)
target_link_libraries(Test2 Resource ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})

install(TARGETS Test2 RUNTIME DESTINATION ${BIN_DIR})
