cmake_minimum_required(VERSION 3.22)
project(cooper-im-server)

set(CMAKE_CXX_STANDARD 17)
set(COOPER_TEST OFF)

add_subdirectory(third_party/cooper)

add_executable(cooper-im-server
        main.cpp
        # entity
        entity/Entity.hpp
        # service
        service/UserService.cpp
        service/UserService.hpp
        # define
        define/IMDefine.hpp
        # store
        store/IMStore.cpp
        store/IMStore.hpp)
add_executable(add-test-data
        test/AddTestData.cpp)

target_link_libraries(cooper-im-server cooper)
target_link_libraries(add-test-data cooper)

target_include_directories(cooper-im-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(add-test-data PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
