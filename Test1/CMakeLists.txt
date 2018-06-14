project(Test1)
add_executable(Test1 src/main.cpp)
target_link_libraries(Test1 ${SDL2_LIBRARY} Resource)

install(TARGETS Test1 RUNTIME DESTINATION ${BIN_DIR})
