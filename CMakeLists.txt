cmake_minimum_required(VERSION 3.21)
project(SimpleExchanger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.40 COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_subdirectory(import/json)
add_subdirectory(src/server)
add_subdirectory(src/client)
