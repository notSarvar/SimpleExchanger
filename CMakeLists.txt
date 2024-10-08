cmake_minimum_required(VERSION 3.21)
project(SimpleExchanger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -std=c++17)
add_link_options(-Wall -Wextra -Wpedantic -Werror)

find_package(Boost 1.40 COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
include_directories(${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/src/server/include/ ${PROJECT_SOURCE_DIR}/import/json)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_subdirectory(src/server)
add_subdirectory(import/json)

file(GLOB_RECURSE SERVER src/server/server.cpp)
file(GLOB_RECURSE CLIENT src/client/*.cpp)

add_executable(Server ${SERVER})
add_executable(Client ${CLIENT})

TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} Core Common Json)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})