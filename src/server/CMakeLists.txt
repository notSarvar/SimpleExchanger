project(Server)

include_directories(core/include)

add_subdirectory(core)

add_executable(Server 
    Server.cpp
)

target_include_directories(Server PUBLIC core/src)
target_link_libraries(Server PUBLIC Core)
