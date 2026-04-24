project(scanner_cli)

add_executable(${PROJECT_NAME}
	scanner_cli.cpp)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	scanner)
