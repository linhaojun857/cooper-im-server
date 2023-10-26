cmake_minimum_required(VERSION 3.22)
project(cooper-im-server)

set(CMAKE_CXX_STANDARD 17)
set(COOPER_TEST OFF)

add_subdirectory(third_party/cooper)

find_package(jwt-cpp CONFIG REQUIRED)

add_library(cooper-im-server-lib
        main.cpp
        # entity
        entity/Entity.hpp
        # controller
        controller/UserController.hpp
        controller/UserController.cpp
        controller/MsgController.hpp
        controller/MsgController.cpp
        # define
        define/IMDefine.hpp
        # store
        store/IMStore.cpp
        store/IMStore.hpp
        # util
        util/JwtUtil.cpp
        util/JwtUtil.hpp)
target_link_libraries(cooper-im-server-lib PUBLIC cooper)
target_link_libraries(cooper-im-server-lib PUBLIC jwt-cpp::jwt-cpp)
target_include_directories(cooper-im-server-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_path(HIREDIS_HEADER hiredis)
target_include_directories(cooper-im-server-lib PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
target_link_libraries(cooper-im-server-lib PUBLIC ${HIREDIS_LIB})

find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(cooper-im-server-lib PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(cooper-im-server-lib PUBLIC ${REDIS_PLUS_PLUS_LIB})

add_executable(cooper-im-server main.cpp)
target_link_libraries(cooper-im-server PRIVATE cooper-im-server-lib)

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
foreach (TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE cooper-im-server-lib)
endforeach ()
