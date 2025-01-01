cmake_minimum_required(VERSION 3.29)
project(duplicate_extraction)

set(CMAKE_CXX_STANDARD 26)

add_library(duplicate_extraction STATIC library.cpp)
