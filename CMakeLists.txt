cmake_minimum_required(VERSION 4.3)
project(compiler)

add_library(defs INTERFACE)
target_include_directories(defs INTERFACE include)

add_library(scanner
	include/scanner.h
	src/scanner.cpp)
target_include_directories(scanner PUBLIC include)

add_subdirectory(src)
add_subdirectory(tools)
