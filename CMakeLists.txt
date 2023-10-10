cmake_minimum_required(VERSION 3.22)
project(server-template)

set(CMAKE_CXX_STANDARD 17)
set(COOPER_TEST OFF)

add_subdirectory(third_party/cooper)

add_executable(server-template main.cpp)
target_link_libraries(server-template cooper)
