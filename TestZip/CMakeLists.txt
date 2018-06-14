project(TestZip)
add_executable(TestZip src/main.cpp)
target_link_libraries(TestZip Resource ZipLoader)

install(TARGETS TestZip RUNTIME DESTINATION ${BIN_DIR})