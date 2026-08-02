#ifndef LEXER_H
#define LEXER_H

#include "defs.h"
#include <stddef.h>

#define LEXER_LOG(lexer, fmt, ...)     LOG("[%d:%d] " fmt, (lexer)->pos.row, (lexer)->pos.col, ##__VA_ARGS__)
#define LEXER_LOG_DBG(lexer, fmt, ...) LOG_DBG("[%d:%d] " fmt, (lexer)->pos.row, (lexer)->pos.col, ##__VA_ARGS__)
#define LEXER_LOG_ERR(lexer, fmt, ...) LOG_ERR("[%d:%d] " fmt, (lexer)->pos.row, (lexer)->pos.col, ##__VA_ARGS__)

typedef struct
{
	int row;
	int col;
} FilePosition;

typedef struct
{
	const char *file_contents;
	size_t file_size;
	FilePosition pos;
	
	char cur_char;
	int cur_pos;
	int last_accepting_pos;

	int lexeme_start;
	Category category;
} Lexer;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);
void lex(Lexer *lexer, Arena *arena, size_t *count_out);

#endif /* LEXER_H */
