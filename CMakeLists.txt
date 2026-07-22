cmake_minimum_required(VERSION 4.3)
project(compiler C)

add_library(defs INTERFACE)
target_include_directories(defs INTERFACE include)

add_library(lexer
	include/lexer.h
	src/lexer.c)
target_include_directories(lexer PUBLIC include)

add_subdirectory(tools)
