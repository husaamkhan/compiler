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

// TODO: Current character was changed to a global variable, meaning it does not
// need to be returned. Instead of returning '\0' which doesn't seem very reliable
// to me, we can return a status code to track EOF
static char next_char(Lexer *lexer)
{
	if (lexer->cur_pos >= (int)lexer->file_size)
	{
		LOG_DBG("End of file detected at position %d", lexer->cur_pos);
		lexer->cur_char = '\0';
		return lexer->cur_char;
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

	return lexer->cur_char;
}

static void handle_character(Lexer *lexer)
{
	int remaining = (int)lexer->file_size - lexer->cur_pos;

	if (remaining >= 5 && strncmp(lexer->file_contents + lexer->cur_pos, "space", 5) == 0)
	{
		LEXER_LOG_DBG(lexer, "character token: 'space'");
		lexer->cur_pos += 5;
	}
	else if (remaining >= 7 && strncmp(lexer->file_contents + lexer->cur_pos, "newline", 7) == 0)
	{
		LEXER_LOG_DBG(lexer, "character token: 'newline'");
		lexer->cur_pos += 7;
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

static char scan_real(Lexer *lexer, char ch)
{
	// TODO: This is an absolutely terrible way to do this and the cyclomatic complexity of this function is way too high.
	// Should probably find a better way to do this
	typedef enum
	{
		SIGN_STATE, DIGITS_STATE, HASHES_STATE, DOT_STATE, DOT_DIGITS_STATE, SLASH_STATE, SLASH_DIGITS_STATE,
		EXP_MARKER_STATE, EXP_SIGN_STATE, EXP_DIGITS_STATE
	} ScanState;

	ScanState state = SIGN_STATE;

	while (1)
	{
		ScanState next_state = state;

		switch (state)
		{
			case SIGN_STATE:
				if (ch == '+' || ch == '-') { next_state = DIGITS_STATE;      break; }
				// fall through for unsigned real
			case DIGITS_STATE:
				if (isdigit(ch))            { next_state = DIGITS_STATE;       lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				if (ch == '#')              { next_state = HASHES_STATE;       lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				if (ch == '.')              { next_state = DOT_STATE;          break; }
				if (ch == '/')              { next_state = SLASH_STATE;        break; }
				if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
				                            { next_state = EXP_MARKER_STATE;   break; }
				return ch;

			case HASHES_STATE:
				if (ch == '#')              { break; }
				if (ch == '.')              { next_state = DOT_STATE;          break; }
				if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
				                            { next_state = EXP_MARKER_STATE;   break; }
				return ch;

			case DOT_STATE:
			case DOT_DIGITS_STATE:
				if (isdigit(ch))            { next_state = DOT_DIGITS_STATE;   lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				if (ch == '#')              { next_state = HASHES_STATE;       lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
				                            { next_state = EXP_MARKER_STATE;   break; }
				return ch;

			case SLASH_STATE:
			case SLASH_DIGITS_STATE:
				if (isdigit(ch))            { next_state = SLASH_DIGITS_STATE;  lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				if (ch == '#')              { next_state = HASHES_STATE;        lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				return ch;

			case EXP_MARKER_STATE:
			case EXP_SIGN_STATE:
				if (ch == '+' || ch == '-') { next_state = EXP_SIGN_STATE;     break; }
				// fall through
			case EXP_DIGITS_STATE:
				if (isdigit(ch))            { next_state = EXP_DIGITS_STATE;   lexer->category = NUMBER; lexer->last_accepting_pos = lexer->cur_pos; break; }
				return ch;
		}

		state = next_state;
		ch = next_char(lexer);
	}
}

static void handle_number(Lexer *lexer)
{
	next_char(lexer);

	// Scan <real R> = <sign_STATE> <ureal R> using a state machine
	// States reflect what characters are valid after each symbol consumed
	// Returns the next unprocessed character
	LEXER_LOG_DBG(lexer, "scanning real number");
	lexer->cur_char = scan_real(lexer, lexer->cur_char);

	if (lexer->cur_char == '@')
	{
		// Polar complex: <real R> @ <real R>
		LEXER_LOG_DBG(lexer, "scanning polar complex");
		next_char(lexer);
		lexer->cur_char = scan_real(lexer, lexer->cur_char);
	}
	else if (lexer->cur_char == '+' || lexer->cur_char == '-')
	{
		// Rectangular complex: <real R> +/- <ureal R> i  or  <real R> +/- i
		LEXER_LOG_DBG(lexer, "scanning rectangular complex");
		next_char(lexer);
		lexer->cur_char = scan_real(lexer, lexer->cur_char);
	}

	// In R5RS, 'i' can only appear as the last character of a number
	if (lexer->cur_char == 'i')
	{
		lexer->category = NUMBER;
		lexer->last_accepting_pos = lexer->cur_pos;
	}
}

static void handle_number_prefix(Lexer *lexer)
{
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

static void handle_identifier(Lexer *lexer, char ch)
{
	// Identifiers are not allowed to start with a digit
	if (isdigit(ch))
	{
		LEXER_LOG_ERR(lexer, "identifier cannot start with a digit");
		return;
	}

	lexer->category = IDENTIFIER;
	lexer->last_accepting_pos = lexer->cur_pos;

	while (1)
	{
		char c = next_char(lexer);
		if (c == ' ' || c == '\n' || c == '\0' || c == '#' || c == '|' || c == '`' || c == '\\' ||
			c == '\'' || c == ';' || c == ',' || c == '(' || c == ')')
		{
			break;
		}
		else
		{
			LEXER_LOG_DBG(lexer, "valid identifier");
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

		// Strings can have any character except for " and \, unless they form \" or \\
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

	// TODO: We break from the loop on \0, which means that the function will end up accepting an
	// unclosed string
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

			if (isdigit(lexer->cur_char))
			{
				handle_number(lexer);
				break;
			}

			handle_identifier(lexer, lexer->cur_char);
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
