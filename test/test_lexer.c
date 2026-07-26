#include "unity.h"
#include "defs.h"
#include "lexer.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_empty_file(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "", 0);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(1, token_count);
	TEST_ASSERT_EQUAL_INT(END_OF_FILE, tokens[0].category);
	TEST_ASSERT_NULL(tokens[0].lexeme);
	TEST_ASSERT_EQUAL_size_t(0, tokens[0].lexeme_len);

	arena_destroy(&arena);
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_empty_file);

	return UNITY_END();
}
