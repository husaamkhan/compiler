#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

// TODO: This is not allowing any of the hexadecimal letters
static void handle_digits(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing digits");

	while (isdigit(lexer->cur_char)) 
	{
		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);
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
static void handle_real(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing real number");

	if (lexer->cur_char == '+' || lexer->cur_char == '-') next_char(lexer);
	
	handle_digits(lexer);

	if (lexer->cur_char == '.')
	{
		lexer->last_accepting_pos = lexer->cur_pos;
		next_char(lexer);

		handle_digits(lexer);
		handle_unspecified_digits(lexer);
		return;
	}

	handle_unspecified_digits(lexer);

	if (lexer->cur_char == '/')
	{
		next_char(lexer);
		handle_digits(lexer);
		handle_unspecified_digits(lexer);
	}
}

static void handle_number(Lexer *lexer)
{
	LEXER_LOG_DBG(lexer, "lexing number");

	lexer->category = NUMBER;
	handle_real(lexer);
}

static void handle_number_prefix(Lexer *lexer)
{
	lexer->category = NUMBER;

	// Optional second prefix component (#e/#i/#b/#o/#d/#x)
	// Peek ahead to check without consuming
	if (lexer->cur_pos < (int)lexer->file_size && lexer->file_contents[lexer->cur_pos] == '#')
	{
		LEXER_LOG_DBG(lexer, "second number prefix found");
		next_char(lexer); // consume '#'
		next_char(lexer); // consume second prefix char (e/i/b/o/d/x)
		if (lexer->cur_char != 'i' && lexer->cur_char != 'e' && lexer->cur_char != 'b' &&
			lexer->cur_char != 'o' && lexer->cur_char != 'd' && lexer->cur_char != 'x')
		{
			LEXER_LOG_ERR(lexer, "invalid number prefix");
			return;
		}
	}

	next_char(lexer); // advance to first number character
	handle_number(lexer);
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

		case 'i': // -+ Handle exactness
		case 'e': // -+
		case 'b': // -+ Handle radices
		case 'o': //  |
		case 'd': //  |
		case 'x': // -+
			// In the case where a # is read, the prefix of a number can start with either
			// the exactness or the radix. The possible options for these are:
			// Exactness: #i, #e | Radix: #b, #o, #d, #x
			handle_number_prefix(lexer);
			break;

		default:
			LEXER_LOG_ERR(lexer, "unrecognized character after '#': '%c'", lexer->cur_char);
			break;
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
					handle_number(lexer);
					break;
				}
			}

			else if (isdigit(lexer->cur_char))
			{
				handle_number(lexer);
				break;
			}

			else if (lexer->cur_char == '.')
			{
				next_char(lexer);
				handle_number(lexer);
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
