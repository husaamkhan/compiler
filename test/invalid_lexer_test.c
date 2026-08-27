//-------------------------------------+
//	invalid_lexer_test.c		|
//-------------------------------------+
// Tests single token output for	|
// invalid input cases.			|
//-------------------------------------+

#include "unity.h"
#include "defs.h"
#include "lexer.h"

// TODO: Modify lexer lex to just output the first available token, or an error,
// rather than internally populate the arena. This will make testing much easier.
// Still need to figure out how we will assert for the exact error though. Maybe
// error enums? I don't know yet, but we need a way to get exactly what error the
// lexer ran into and assert for that in the test.
// NOTE: Doing this would mean that I need to go back to valid_lexer_test.c and
// change it to not expect END_OF_FILE tokens
test_invalid_character(void)
{
	Arena arena = arena_create(sizeof(Token));
	Lexer lexer;
	lexer_init(&lexer, "#/aa", 0);

	size_t token_count = 0;
	lex(&lexer, &arena, &token_count);

	assert_eq(token_count, 0);
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_invalid_character);

	return UNITY_END();
}
