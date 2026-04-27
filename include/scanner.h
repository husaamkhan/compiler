#include "defs.h"
#include <iostream>
#include <queue>
#include <stack>

#ifndef SCANNER
#define SCANNER

#define BUFFER_LENGTH 4096

enum State { ERROR, START, ACCEPTING, NON_ACCEPTING };

struct CategoryStatePair
{
	State state = ERROR;
	Category category = NONE;
};

class Scanner
{ 
	public:
		Scanner();
		~Scanner();

		void scan();
		void init(std::istream* in, std::queue<Token>* out);

	private:
		void fillBuffer(int pos);
		char nextChar();
		void rollBack();
		CategoryStatePair handleHash();
		Token getNextToken();

		std::istream* input_stream;
		std::queue<Token>* output;

		char double_buffer[2 * BUFFER_LENGTH];
		int lexeme_start;
		int cur_pos;
		int fence; // Used to prevent rollback into incorrect buffer

		std::stack<CategoryStatePair> stack;
};
#endif
