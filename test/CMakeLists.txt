add_executable(test_lexer test_lexer.c)

target_link_libraries(test_lexer
	lexer
	Unity)

add_test(test_lexer test_lexer)