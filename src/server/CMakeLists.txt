project(Server)

add_subdirectory(core)

add_executable(Server Server.cpp)

target_include_directories(Server PUBLIC core/include)
target_include_directories(Server PUBLIC core/src)
target_link_libraries(Server PUBLIC Json)