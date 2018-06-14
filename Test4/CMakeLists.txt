project(Test4)
add_executable(Test4 src/main.cpp)
target_link_libraries(Test4 Resource ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARY})

install(TARGETS Test4 RUNTIME DESTINATION ${BIN_DIR})
