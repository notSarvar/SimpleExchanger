cmake_minimum_required(VERSION 3.21)

file(GLOB_RECURSE COMMON_SRC *.hpp *.h *.cpp)

add_library(Common INTERFACE)

set_target_properties(Common PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Common INTERFACE ${PROJECT_SOURCE_DIR})
