project(Server)

cmake_minimum_required(VERSION 3.21)

add_subdirectory(core)

add_executable(Server Server.cpp)