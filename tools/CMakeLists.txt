project(lexer_cli C)

add_executable(${PROJECT_NAME}
	lexer_cli.c)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	defs
	lexer)
