#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

static void print_usage(void)
{
	fprintf(stderr, "Usage:\n"
		"./lexer_cli              -    Enter text interactively\n"
		"./lexer_cli <file>       -    Read input from file\n");
}

static void print_tokens(Token *tokens, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		const char *category = "";
		switch (tokens[i].category)
		{
			case BOOLEAN:
				category = "BOOLEAN";
				break;

			case CHARACTER:
				category = "CHARACTER";
				break;

			case IDENTIFIER:
				category = "IDENTIFIER";
				break;

			case STRING:
				category = "STRING";
				break;

			case WHITESPACE:
				category = "WHITESPACE";
				if (tokens[i].lexeme[0] == '\n')
				{
					 tokens[i].lexeme = "\\n";
					 tokens[i].lexeme_len = 2;
				}
				break;

			case END_OF_FILE:
				category = "END_OF_FILE";
				break;

			case NUMBER:
				category = "NUMBER";
				break;

			default:
				category = "NONE";
				break;
		}

		printf("[%.*s: %s]\n", (int)tokens[i].lexeme_len, tokens[i].lexeme, category);
	}
}

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		print_usage();
		return 1;
	}

	char *file_contents = NULL;
	size_t file_size = 0;

	if (argc == 1)
	{
		// Read from stdin into a dynamically grown buffer
		size_t capacity = 4096;
		file_contents = malloc(capacity);
		int c;

		while ((c = fgetc(stdin)) != EOF)
		{
			if (file_size >= capacity)
			{
				capacity *= 2;
				file_contents = realloc(file_contents, capacity);
			}
			file_contents[file_size++] = (char)c;
		}
	}
	else
	{
		FILE *file = fopen(argv[1], "r");
		if (file == NULL)
		{
			fprintf(stderr, "Could not open file: %s\n", argv[1]);
			return 1;
		}

		file_contents = read_file(file, &file_size);
		fclose(file);

		if (file_contents == NULL)
		{
			return 1;
		}
	}

	Lexer lexer;
	lexer_init(&lexer, file_contents, file_size);

	Arena arena = arena_create(sizeof(Token) * 64);
	size_t count = 0;
	lex(&lexer, &arena, &count);

	print_tokens((Token *)arena.base, count);

	arena_destroy(&arena);
	free(file_contents);

	return 0;
}
