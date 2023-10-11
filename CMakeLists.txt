cmake_minimum_required(VERSION 3.22)
project(cooper-im-server)

set(CMAKE_CXX_STANDARD 17)
set(COOPER_TEST OFF)

add_subdirectory(third_party/cooper)

add_executable(cooper-im-server
        main.cpp
        entity/Entity.hpp
        service/UserService.cpp
        service/UserService.hpp
        define/IMDefine.hpp)

target_link_libraries(cooper-im-server cooper)

target_include_directories(cooper-im-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
