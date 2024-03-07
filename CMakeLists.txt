cmake_minimum_required(VERSION 3.10)

project(meowchkin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

find_package(Boost 1.72.0 COMPONENTS system asio)
include_directories(${Boost_INCLUDE_DIRS})

include_directories("include/"
        "third_party/")

FILE(GLOB SRCFILES src/*.cpp include/*.h)

add_executable(server_example examples/server_example.cpp ${SRCFILES})
add_executable(client_example examples/client_example.cpp  ${SRCFILES})

target_link_libraries(server_example ${Boost_LIBRARIES})
target_link_libraries(client_example ${Boost_LIBRARIES})
