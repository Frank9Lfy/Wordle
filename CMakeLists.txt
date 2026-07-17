cmake_minimum_required(VERSION 4.0)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(untitled main.cpp)
target_link_libraries(untitled ${CMAKE_SOURCE_DIR}/lib/libeasyx.a)
file(COPY ${CMAKE_SOURCE_DIR}/words.txt DESTINATION ${CMAKE_BINARY_DIR})