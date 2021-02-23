cmake_minimum_required(VERSION 3.19.3)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "")

project(webserv)

add_executable(webserv webserv.cpp)
