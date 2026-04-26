#include <iostream>

#ifndef SCANNER
#define SCANNER

#define BUFFER_LENGTH 4096

enum State { ERROR, ACCEPTING, NON_ACCEPTING };
enum Token
{
	INCOMPLETE,
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
	DOT
};

class Scanner
{ 
	public:
		Scanner();
		~Scanner();

		void scan();
		void init(std::istream* stream);

	private:
		void fillBuffer(int pos);
		char nextChar();

		std::istream* input_stream;

		char double_buffer[2 * BUFFER_LENGTH];
		int cur_pos;
		int fence; // Used to prevent rollback into incorrect buffer


		int last_accepting_pos;
};
#endif
