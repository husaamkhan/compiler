#include "unity.h"
#include <string.h>
#include "defs.h"
#include "lexer.h"

#define LOG_TEST(fmt, ...) printf("[TEST] " fmt "\n", ##__VA_ARGS__);

void setUp(void)
{
	LOG("\n--- Running test: %s ---", Unity.CurrentTestName);
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
}

void test_boolean_true(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	char s[] = "#t";
	lexer_init(&lexer, s, 2);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(BOOLEAN, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
}

void test_boolean_false(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	char s[] = "#f";
	lexer_init(&lexer, s, 2);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(BOOLEAN, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
}

void test_character_space(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	char s[] = "#\\space";
	lexer_init(&lexer, s, 7);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
}

void test_character_newline(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	
	char s[] = "#\\newline";
	lexer_init(&lexer, s, 9);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
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

		lexer_init(&lexer, s, 4);

		size_t token_count = 0;
		lex(&lexer, &arena, &token_count);

		Token *tokens = (Token *)arena.base;

		TEST_ASSERT_EQUAL_size_t(2, token_count);
		TEST_ASSERT_EQUAL_INT(CHARACTER, tokens[0].category);
		TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
		TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);

		test_char++;
	}
}

void test_integers(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	LOG_TEST("Testing integer: 1");
	char *s = "1";
	lexer_init(&lexer, s, 2);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);

	LOG_TEST("Testing integer: 52");
	s = "52";
	lexer_init(&lexer, s, 3);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);

	LOG_TEST("Testing integer: 0015");
	s = "0015";
	lexer_init(&lexer, s, 5);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);

	LOG_TEST("Testing integer: 2345");
	s = "2345";
	lexer_init(&lexer, s, 5);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
}

void test_integers_with_hash(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;

	LOG_TEST("Testing integer: 1##");
	char *s = "1##";
	lexer_init(&lexer, s, 4);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);


	LOG_TEST("Testing integer: 52#####");
	s = "52#####";
	lexer_init(&lexer, s, 8);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
	
	LOG_TEST("Testing integer: 0015##");
	s = "0015##";
	lexer_init(&lexer, s, 7);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);

	LOG_TEST("Testing integer: 2345#");
	s = "2345#";
	lexer_init(&lexer, s, 6);

	token_count = 0;
	arena.used = 0;
	lex(&lexer, &arena, &token_count);

	tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(NUMBER, tokens[0].category);
	TEST_ASSERT_EQUAL_STRING(s, tokens[0].lexeme);
	TEST_ASSERT_EQUAL_INT(strlen(s), tokens[0].lexeme_len);
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
	RUN_TEST(test_integers);
	RUN_TEST(test_integers_with_hash);

	return UNITY_END();
}
