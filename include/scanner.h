#include "defs.h"
#include <iostream>
#include <queue>

#ifndef SCANNER
#define SCANNER

#define BUFFER_LENGTH 4096

class Scanner
{ 
	public:
		void init(std::istream *in);
		void scan(std::queue<Token>* output);

	private:
		char nextChar();
		void rollBack();
		Token getNextToken();

		void handleHash();
		void handleCharacter();
		void handleIdentifier(char ch);
		void handleString();
		void handleNumber();

		std::vector<char> file_contents;

		int lexeme_start;
		int cur_pos;
		int last_accepting_pos;
		Category category;
};
#endif
