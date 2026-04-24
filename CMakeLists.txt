project(compiler)

add_library(scanner
	include/scanner.h
	src/scanner.cpp)

target_include_directories(scanner PUBLIC include)

add_subdirectory(src)
add_subdirectory(tools)
