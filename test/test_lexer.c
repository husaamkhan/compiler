#include "unity.h"
#include <string.h>
#include "defs.h"
#include "lexer.h"

#define LOG_TEST(fmt, ...) printf("[TEST] " fmt "\n", ##__VA_ARGS__);

void setUp(void)
{
	LOG("--- Running test: %s ---", Unity.CurrentTestName);
}

void tearDown(void)
{}

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

void test_boolean_true(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "#t", 2);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(BOOLEAN, tokens[0].category);
	TEST_ASSERT_EQUAL(strcmp("#t", tokens[0].lexeme), 0);
	TEST_ASSERT_EQUAL_INT(2, tokens[0].lexeme_len);

	arena_destroy(&arena);
}

void test_boolean_false(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "#f", 2);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(BOOLEAN, tokens[0].category);
	TEST_ASSERT_EQUAL(strcmp("#f", tokens[0].lexeme), 0);
	TEST_ASSERT_EQUAL_INT(2, tokens[0].lexeme_len);

	arena_destroy(&arena);
}

void test_character_space(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "#\\space", 7);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
	TEST_ASSERT_EQUAL(strcmp("#\\space", tokens[0].lexeme), 0);
	TEST_ASSERT_EQUAL_INT(7, tokens[0].lexeme_len);

	arena_destroy(&arena);
}

void test_character_newline(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "#\\newline", 9);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
	TEST_ASSERT_EQUAL(strcmp("#\\newline", tokens[0].lexeme), 0);
	TEST_ASSERT_EQUAL_INT(9, tokens[0].lexeme_len);

	arena_destroy(&arena);
}

void test_characters(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	// Iterate through all printable ascii characters
	// Scheme does not support extended ascii characters
	char test_char = '!';
	for (int i = 33; i < 127; i++)
	{
		char s[4] = "#\\";
		strncat(s, &test_char, 1);

		LOG_TEST("Testing character '%s'", s);

		lexer_init(&lexer, s, 1);

		size_t token_count = 0;
		lex(&lexer, &arena, &token_count);

		Token *tokens = (Token *)arena.base;

		TEST_ASSERT_EQUAL_size_t(2, token_count);
		TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
		TEST_ASSERT_EQUAL(strcmp(s, tokens[0].lexeme), 0);
		TEST_ASSERT_EQUAL_INT(3, tokens[0].lexeme_len);

		test_char++;
	}
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_empty_file);
	RUN_TEST(test_boolean_true);
	RUN_TEST(test_boolean_false);
	RUN_TEST(test_character_space);
	RUN_TEST(test_character_newline);
	RUN_TEST(test_characters);

	return UNITY_END();
}
