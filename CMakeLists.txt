cmake_minimum_required(VERSION 3.16)
project(ChatTCP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)

add_subdirectory(src/)
