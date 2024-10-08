cmake_minimum_required(VERSION 3.21)

add_library(Json INTERFACE)

target_include_directories(Json INTERFACE {CMAKE_CURRENT_SOURCE_DIR})