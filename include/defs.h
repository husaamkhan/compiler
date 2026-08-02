#ifndef DEFS_H
#define DEFS_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define LOG(fmt, ...)     fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
    #define LOG_DBG(fmt, ...) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_DBG(fmt, ...) ((void)0)
#endif

typedef enum
{
	NONE,
	IDENTIFIER,
	BOOLEAN,
	NUMBER,
	CHARACTER,
	STRING,
	L_PARENTHESIS,
	R_PARENTHESIS,
	VECTOR_START,
	QUOTE,
	QUASIQUOTE,
	UNQUOTE,
	UNQUOTE_SPLICE,
	DOT,
	NEWLINE,
	WHITESPACE,
	END_OF_FILE
} Category;

typedef struct
{
	Category category;
	const char *lexeme;
	size_t lexeme_len;
} Token;

typedef struct
{
	char *base;
	size_t used;
	size_t capacity;
} Arena;

static Arena arena_create(size_t initial_capacity)
{
	Arena arena;
	arena.base = malloc(initial_capacity);
	arena.used = 0;
	arena.capacity = initial_capacity;
	return arena;
}

static inline void *arena_alloc(Arena *arena, size_t size)
{
	if (arena->used + size > arena->capacity)
	{
		arena->capacity *= 2;
		arena->base = realloc(arena->base, arena->capacity);
	}

	void *ptr = arena->base + arena->used;
	arena->used += size;
	return ptr;
}

static void arena_destroy(Arena *arena)
{
	free(arena->base);
	arena->base = NULL;
	arena->used = 0;
	arena->capacity = 0;
}

// Wraps arena_alloc to ensure tokens are always allocated with the correct size,
// preventing callers from passing an incorrect size to arena_alloc directly.
#define token_new(arena) ((Token *)arena_alloc(arena, sizeof(Token)))

static char *read_file(FILE *in, size_t *size_out)
{
	fseek(in, 0, SEEK_END);
	*size_out = ftell(in);
	fseek(in, 0, SEEK_SET);

	char *buffer = malloc(*size_out);

	if (!fread(buffer, 1, *size_out, in))
	{
		fprintf(stderr, "Error: couldn't read file\n");
		free(buffer);
		return NULL;
	}

	return buffer;
}

#endif /* DEFS_H */
