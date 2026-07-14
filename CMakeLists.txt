cmake_minimum_required(VERSION 3.16)
project(TCPChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)

# gtest
find_package(GTest REQUIRED)

add_subdirectory(src/)
