//--------------------------------------+
//	valid_lexer_test.c		|
//--------------------------------------+
// Tests single token output for valid	|
// input cases.				|
//--------------------------------------+

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
	LOG_TEST("--- Input: %s ---", input);

	Arena arena = arena_create(sizeof(Token) * 2);
	Lexer lexer;
	lexer_init(&lexer, input, strlen(input) + 1);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	Token *tokens = (Token *)arena.base;

	// Asserting 2 tokens as lexer will output the expected token and the END_OF_FILE token
	TEST_ASSERT_EQUAL_size_t(2, token_count);
	TEST_ASSERT_EQUAL_INT(expected_category, tokens[0].category);
	TEST_ASSERT_EQUAL_size_t(strlen(input), tokens[0].lexeme_len);
}

void test_empty_file(void)
{
	// Not using assert_token here since it expects 2 tokens, one for EOF and another for
	// the provided token, whereas this function asserts token_count == 1
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

void test_signed_integers(void)
{
	assert_token("+3",    NUMBER);
	assert_token("-42",   NUMBER);
	assert_token("+0015", NUMBER);
	assert_token("-2345", NUMBER);
}

void test_signed_reals(void)
{
	assert_token("+1.0",  NUMBER);
	assert_token("-3.14", NUMBER);
	assert_token("+0.5",  NUMBER);
}

void test_signed_fractions(void)
{
	assert_token("+1/2",  NUMBER);
	assert_token("-22/7", NUMBER);
}

void test_exactness_prefix(void)
{
	assert_token("#e1",   NUMBER);
	assert_token("#i1",   NUMBER);
	assert_token("#e3.14", NUMBER);
	assert_token("#i1/2", NUMBER);
}

void test_base_prefix_decimal(void)
{
	assert_token("#d42",  NUMBER);
	assert_token("#d3.14", NUMBER);
	assert_token("#d1/2", NUMBER);
}

void test_two_part_prefix(void)
{
	assert_token("#e#b101", NUMBER);
	assert_token("#i#o77",  NUMBER);
	assert_token("#e#x1F",  NUMBER);
	assert_token("#b#e101", NUMBER);
	assert_token("#x#i1F",  NUMBER);
}

void test_dot_prefix(void)
{
	assert_token(".5",  NUMBER);
	assert_token(".14", NUMBER);
}

void test_rectangular_complex(void)
{
	assert_token("3+4i",  NUMBER);
	assert_token("1-2i",  NUMBER);
	assert_token("0+1i",  NUMBER);
	assert_token("-1+2i", NUMBER);
}

void test_polar_complex(void)
{
	assert_token("1@2",     NUMBER);
	assert_token("3.0@1.5", NUMBER);
	assert_token("1/2@3/4", NUMBER);
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
	RUN_TEST(test_signed_integers);
	RUN_TEST(test_signed_reals);
	RUN_TEST(test_signed_fractions);
	RUN_TEST(test_exactness_prefix);
	RUN_TEST(test_base_prefix_decimal);
	RUN_TEST(test_two_part_prefix);
	RUN_TEST(test_dot_prefix);
	RUN_TEST(test_rectangular_complex);
	RUN_TEST(test_polar_complex);

	return UNITY_END();
}
