project(Server)

include_directories(core/include)

add_subdirectory(core)

add_executable(Server Server.cpp)

target_include_directories(Server PUBLIC core/src)
