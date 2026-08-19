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

static void assert_token(const char *input, Category expected_category)
{
	Arena arena = arena_create(sizeof(Token) * 2);
	Lexer lexer;
	lexer_init(&lexer, input, strlen(input) + 1);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(expected_category, tokens[0].category);
	TEST_ASSERT_EQUAL_size_t(strlen(input), tokens[0].lexeme_len);
}

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
	assert_token("#t", BOOLEAN);
}

void test_boolean_false(void)
{
	assert_token("#f", BOOLEAN);
}

void test_character_space(void)
{
	assert_token("#\\space", CHARACTER);
}

void test_character_newline(void)
{
	assert_token("#\\newline", CHARACTER);
	
}

void test_characters(void)
{
	// Iterate through all printable ASCII characters
	// Scheme does not support extended ASCII characters
	char test_char = '!';
	for (int i = 33; i < 127; i++)
	{
		char s[4] = "#\\";
		strncat(s, &test_char, 1);

		LOG_TEST("Testing character '%s'", s);
		
		assert_token(s, CHARACTER);
		test_char++;
	}
}

void test_binary_integers(void)
{
	assert_token("#b1",        NUMBER);
	assert_token("#b0",        NUMBER);
	assert_token("#b1010",     NUMBER);
	assert_token("#b00001111", NUMBER);
}

void test_binary_integers_with_hash(void)
{
	assert_token("#b1##",     NUMBER);
	assert_token("#b1010###", NUMBER);
}

void test_octal_integers(void)
{
	assert_token("#o7",        NUMBER);
	assert_token("#o0",        NUMBER);
	assert_token("#o1234567",  NUMBER);
	assert_token("#o0077",     NUMBER);
}

void test_octal_integers_with_hash(void)
{
	assert_token("#o7##",     NUMBER);
	assert_token("#o1234###", NUMBER);
}

void test_decimal_integers(void)
{
	assert_token("1",    NUMBER);
	assert_token("52",   NUMBER);
	assert_token("0015", NUMBER);
	assert_token("2345", NUMBER);
}

void test_decimal_integers_with_hash(void)
{
	assert_token("1##",      NUMBER);
	assert_token("52#####",  NUMBER);
	assert_token("0015##",   NUMBER);
	assert_token("2345#",    NUMBER);
}

void test_hexadecimal_integers(void)
{
	assert_token("#xFF",       NUMBER);
	assert_token("#x0",        NUMBER);
	assert_token("#x1A2B3C",   NUMBER);
	assert_token("#xdeadbeef", NUMBER);
}

void test_hexadecimal_integers_with_hash(void)
{
	assert_token("#xFF##",    NUMBER);
	assert_token("#x1A2B###", NUMBER);
}

void test_decimal_point(void)
{
	assert_token("1.0",   NUMBER);
	assert_token("3.14",  NUMBER);
	assert_token("0.5##", NUMBER);
}

void test_fractions(void)
{
	assert_token("1/2",   NUMBER);
	assert_token("22/7",  NUMBER);
	assert_token("1/3##", NUMBER);
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
	RUN_TEST(test_binary_integers);
	RUN_TEST(test_binary_integers_with_hash);
	RUN_TEST(test_octal_integers);
	RUN_TEST(test_octal_integers_with_hash);
	RUN_TEST(test_decimal_integers);
	RUN_TEST(test_decimal_integers_with_hash);
	RUN_TEST(test_hexadecimal_integers);
	RUN_TEST(test_hexadecimal_integers_with_hash);
	RUN_TEST(test_decimal_point);
	RUN_TEST(test_fractions);

	return UNITY_END();
}
