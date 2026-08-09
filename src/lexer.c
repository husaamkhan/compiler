#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum
{
	BASE_BINARY,
	BASE_OCTAL,
	BASE_DECIMAL,
	BASE_HEXADECIMAL
} NumberBase;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size)
{
	LOG_DBG("Initializing lexer.");

	lexer->file_contents = file_contents;
	lexer->file_size = file_size;

	lexer->pos.row = 1;
	lexer->pos.col = 0;

	lexer->cur_pos = 0;
	lexer->cur_char = '\0';
	lexer->last_accepting_pos = 0;

	lexer->lexeme_start = 0;
}

// TODO: Need to check if returning \0 may cause any issues in certain cases
static void next_char(Lexer *lexer)
{
	if (lexer->cur_pos >= (int)lexer->file_size)
	{
		LOG_DBG("End of file detected at position %d", lexer->cur_pos);
		lexer->cur_char = '\0';
	}

	lexer->cur_char = lexer->file_contents[lexer->cur_pos];
	lexer->cur_pos++;

	if (lexer->cur_char == '\n')
	{
		lexer->pos.row++;
		lexer->pos.col = 0;
	}
	else
	{
		lexer->pos.col++;
	}
}

static void handle_character(Lexer *lexer)
{
	int remaining = (int)lexer->file_size - lexer->cur_pos;

	if (remaining >= 5 && strncmp(lexer->file_contents + lexer->cur_pos, "space", 5) == 0)
	{
		lexer->cur_pos += 5;
		lexer->pos.col += 5;
		LEXER_LOG_DBG(lexer, "character token: 'space'");
	}
	else if (remaining >= 7 && strncmp(lexer->file_contents + lexer->cur_pos, "newline", 7) == 0)
	{
		lexer->cur_pos += 7;
		lexer->pos.col += 7;
		LEXER_LOG_DBG(lexer, "character token: 'newline'");
	}
	else
	{
		next_char(lexer);

		// TODO: Is it possible that the program gets stuck in a loop here? It could
		// be possible that the lexer reaches this point, then jumps back to the
		// last accepting position, only to scan all the way back to this point
		// resulting in a loop
		if (lexer->cur_char == '\0')
		{
			LEXER_LOG_ERR(lexer, "unexpected end of file");
			return;
		}
	}

	lexer->last_accepting_pos = lexer->cur_pos;
	lexer->category = CHARACTER;
}

static void handle_digits_base_2(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing digits for binary number");

	while (isdigit(lexer->cur_char))
	{
		if (lexer->cur_char != '0' && lexer->cur_char != '1')
		{
			LEXER_LOG_ERR(lexer, "digit '%c' is out of range for binary number", lexer->cur_char);
			return;
		}

		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
	}
}

static void handle_digits_base_8(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing digits for octal number");

	while (isdigit(lexer->cur_char))
	{
		if (lexer->cur_char > '7')
		{
			LEXER_LOG_ERR(lexer, "digit '%c' is out of range for octal number", lexer->cur_char);
			return;
		}

		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
	}
}

static void handle_digits_base_10(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing digits for decimal number");

	while (isdigit(lexer->cur_char))
	{
		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
	}
}

static void handle_digits_base_16(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing digits for hexadecimal number");

	while (
		isdigit(lexer->cur_char) ||
		lexer->cur_char == 'A' || lexer->cur_char == 'a' ||
		lexer->cur_char == 'B' || lexer->cur_char == 'b' ||
		lexer->cur_char == 'C' || lexer->cur_char == 'c' ||
		lexer->cur_char == 'D' || lexer->cur_char == 'd' ||
		lexer->cur_char == 'E' || lexer->cur_char == 'e' ||
		lexer->cur_char == 'F' || lexer->cur_char == 'f'
	)
	{
		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
	}
}

static void handle_digits(Lexer *lexer, NumberBase base)
{
	switch (base)
	{
		case BASE_BINARY:
			handle_digits_base_2(lexer);
			break;
		case BASE_OCTAL:
			handle_digits_base_8(lexer);
			break;
		case BASE_DECIMAL:
			handle_digits_base_10(lexer);
			break;
		default:
			handle_digits_base_16(lexer);
	}
}

static void handle_unspecified_digits(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing unspecified digits");

	while (lexer->cur_char == '#')
	{
		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
	}
}

// TODO: This does not include any suffixes that are allowed in decimals
static void handle_ureal(Lexer *lexer, NumberBase base)
{
	LEXER_LOG_DBG(lexer, "lexing real number");
	
	handle_digits(lexer, base);

	if (lexer->cur_char == '.')
	{
		if (base != BASE_DECIMAL)
		{
			LEXER_LOG_ERR(lexer, "decimal point is not allowed in non-decimal numbers");
			return;
		}

		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);

		handle_digits(lexer, base);
		handle_unspecified_digits(lexer);
		return;
	}

	handle_unspecified_digits(lexer);

	if (lexer->cur_char == '/')
	{
		if (base != BASE_DECIMAL)
		{
			LEXER_LOG_ERR(lexer, "fractional notation is not allowed in non-decimal numbers");
			return;
		}

		next_char(lexer);
		handle_digits(lexer, base);
		handle_unspecified_digits(lexer);
	}
}

// TODO: Based on the prefix (if provided), this function should be lexing the
// number according to the base
static void handle_number(Lexer *lexer, NumberBase base)
{
	LEXER_LOG_DBG(lexer, "lexing number");

	lexer->category = NUMBER;

	if (lexer->cur_char == '+' || lexer->cur_char == '-') next_char(lexer);
	handle_ureal(lexer, base);

	if (lexer->cur_char == '@')
	{
		next_char(lexer);
		if (lexer->cur_char == '+' || lexer->cur_char == '-') next_char(lexer);
		handle_ureal(lexer, base);
	}

	else if (lexer->cur_char == '+' || lexer->cur_char == '-')
	{
		next_char(lexer);
		if (lexer->cur_char == '+' || lexer->cur_char == '-') next_char(lexer);
		handle_ureal(lexer, base);

		if (lexer->cur_char == 'i')
		{
			lexer->last_accepting_pos = lexer->cur_pos;
		}
	}
}

// Only handles the second part of the prefix, if provided
// The first half is handled in handle_hash(), which then calls this function
static void handle_number_prefix(Lexer *lexer)
{
	// In the case where a # is read, the prefix of a number can start with
	// either the exactness or the base. The possible options for these are:
	// Exactness: #i, #e | Base: #b, #o, #d, #x

	NumberBase base;
	bool exactness_provided = false;
	switch(lexer->cur_char)
	{
		case 'i':
		case 'e':
			exactness_provided = true;
				// fallthrough
		case 'd':
			base = BASE_DECIMAL;
			break;
		case 'b':
			base = BASE_BINARY;
			break;
		case 'o':
			base = BASE_OCTAL;
			break;
		case 'x':
			base = BASE_HEXADECIMAL;
			break;
		default:
			return; // invalid character in prefix
	}


	next_char(lexer);

	// In the prefix, the exactness and base can only be given once
	// If another hash is detected, the lexer needs to make sure that the
	// prefix only contains one base and one exactness in any order
	if (lexer->cur_char == '#')
	{

		next_char(lexer);

		if (exactness_provided)
		{
			switch(lexer->cur_char)
			{
				case 'd':
					base = BASE_DECIMAL;
					break;
				case 'b':
					base = BASE_BINARY;
					break;
				case 'o':
					base = BASE_OCTAL;
					break;
				case 'x':
					base = BASE_HEXADECIMAL;
					break;
				default:
					return; // invalid character in prefix
			}
		}

		else
		{
			if (lexer->cur_char != 'i' && lexer->cur_char != 'e')
			{
				return;
			}
		}
	}

	handle_number(lexer, base);
}

// A # can lead to multiple categories. This function checks the next character after a #
// is read to handle booleans, characters, and numbers
static void handle_hash(Lexer *lexer)
{
	next_char(lexer);

	switch (lexer->cur_char)
	{
		case 't':
			lexer->category = BOOLEAN;
			lexer->last_accepting_pos = lexer->cur_pos;
			break;

		case 'f':
			lexer->category = BOOLEAN;
			lexer->last_accepting_pos = lexer->cur_pos;
			break;

		case '\\':
			handle_character(lexer);
			break;

		default:
			handle_number_prefix(lexer);

			// If the category is none at this point, it means the lexer was unable
			// to recognize a valid number prefix and move on to reading a number
			if (lexer->category == NONE)
			{
				LEXER_LOG_ERR(lexer, "unrecognized character after '#': '%c'", lexer->cur_char);
			}
	}
}

static void handle_identifier(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "Handling identifier starting with '%c'", lexer->cur_char);

	lexer->category = IDENTIFIER;
	lexer->last_accepting_pos = lexer->cur_pos;

	while (1)
	{
		next_char(lexer);
		if (lexer->cur_char == ' ' || lexer->cur_char == '\n' || lexer->cur_char == '\0' || lexer->cur_char == '#' || lexer->cur_char == '|' || lexer->cur_char == '`' || lexer->cur_char == '\\' ||
			lexer->cur_char == '\'' || lexer->cur_char == ';' || lexer->cur_char == ',' || lexer->cur_char == '(' || lexer->cur_char == ')')
		{
			break;
		}
		else
		{
			LEXER_LOG_DBG(lexer, "valid identifier: %c", lexer->cur_char);
			lexer->category = IDENTIFIER;
			lexer->last_accepting_pos = lexer->cur_pos;
		}
	}
}

static void handle_string(Lexer *lexer)
{
	next_char(lexer);
	while (lexer->cur_char != '"')
	{
		if (lexer->cur_char == '\0')
		{
			LEXER_LOG_ERR(lexer, "unclosed string");
			break;
		}

		// Strings can have any character except for " and \, unless they form '\"' or '\\'
		// To handle this, the lexer needs to look ahead to see what the next character is
		if (lexer->cur_char == '\\')
		{
			next_char(lexer);
			if (lexer->cur_char != '"' && lexer->cur_char != '\\')
			{
				LEXER_LOG_ERR(lexer, "invalid escape sequence: '\\%c'", lexer->cur_char);
				return;
			}
		}

		next_char(lexer);
	}

	lexer->category = STRING;
	lexer->last_accepting_pos = lexer->cur_pos;
}

static void get_next_token(Lexer *lexer, Token *token)
{
	lexer->category = NONE;
	lexer->lexeme_start = lexer->cur_pos;

	next_char(lexer);
	switch (lexer->cur_char)
	{
		case '\0':
			token->category = END_OF_FILE;
			token->lexeme = NULL;
			token->lexeme_len = 0;
			return;

		case ' ':
			token->category = WHITESPACE;
			token->lexeme = lexer->file_contents + lexer->lexeme_start;
			token->lexeme_len = 1;
			return;

		case '\n':
			token->category = WHITESPACE;
			token->lexeme = lexer->file_contents + lexer->lexeme_start;
			token->lexeme_len = 1;
			return;

		case '#':
			handle_hash(lexer);
			break;

		case '"':
			handle_string(lexer);
			break;

		default:
			if (lexer->cur_char == '+' || lexer->cur_char == '-')
			{
				if (lexer->cur_pos < (int)lexer->file_size && isdigit(lexer->file_contents[lexer->cur_pos]))
				{
					handle_number(lexer, BASE_DECIMAL);
					break;
				}
			}

			else if (isdigit(lexer->cur_char))
			{
				handle_number(lexer, BASE_DECIMAL);
				break;
			}

			else if (lexer->cur_char == '.')
			{
				next_char(lexer);
				handle_number(lexer, BASE_DECIMAL);
			}

			handle_identifier(lexer);
			break;
	}

	lexer->cur_pos = lexer->last_accepting_pos;
	size_t lexeme_len = lexer->cur_pos - lexer->lexeme_start;

	if (lexer->category == NONE)
	{
		LEXER_LOG_ERR(lexer, "unrecognized lexeme: '%.*s'", (int)lexeme_len, lexer->file_contents + lexer->lexeme_start);
		lexer->cur_pos = lexer->lexeme_start + 1;
		lexer->last_accepting_pos = lexer->cur_pos;
		token->category = NONE;
		token->lexeme = lexer->file_contents + lexer->lexeme_start;
		token->lexeme_len = 1;
		return;
	}

	token->category = lexer->category;
	token->lexeme = lexer->file_contents + lexer->lexeme_start;
	token->lexeme_len = lexeme_len;
	LEXER_LOG_DBG(lexer, "token produced: category=%d, lexeme='%.*s'", token->category, (int)token->lexeme_len, token->lexeme);
}

void lex(Lexer *lexer, Arena *arena, size_t *count_out)
{
	*count_out = 0;

	while (1)
	{
		Token *token = token_new(arena);
		get_next_token(lexer, token);

		if (token->category == NONE)
		{
			// Reclaim the slot since we are not keeping this token
			arena->used -= sizeof(Token);
			continue;
		}

		(*count_out)++;

		if (token->category == END_OF_FILE)
		{
			LOG_DBG("Lexing complete. Total tokens: %zu", *count_out);
			break;
		}
	}
}
