#ifndef LEXER_H
#define LEXER_H

#include "defs.h"
#include <stddef.h>

typedef struct
{
	const char *file_contents;
	size_t file_size;
	int lexeme_start;
	int cur_pos;
	int last_accepting_pos;
	Category category;
	char cur_char;
} Lexer;

void lexer_init(Lexer *lexer, const char *file_contents, size_t file_size);
void lex(Lexer *lexer, Arena *arena, size_t *count_out);

#endif /* LEXER_H */
