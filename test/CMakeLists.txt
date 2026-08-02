add_executable(test_lexer test_lexer.c)

target_link_libraries(test_lexer
	lexer
	Unity)

target_compile_definitions(test_lexer PRIVATE DEBUG)

add_test(test_lexer test_lexer)
