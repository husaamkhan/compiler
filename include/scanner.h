#include "defs.h"
#include <iostream>
#include <queue>

#ifndef SCANNER
#define SCANNER

class Scanner
{ 
	public:
		void init(std::istream *in);
		void scan(std::queue<Token>* output);

	private:
		char nextChar();
		std::string lookAhead(int n);
		void rollBack();
		Token getNextToken();

		void handleHash();
		void handleCharacter();
		void handleIdentifier(char ch);
		void handleString();
		void handleNumberPrefix();
		void handleNumber();

		std::vector<char> file_contents;

		int lexeme_start;
		int cur_pos;
		int last_accepting_pos;
		Category category;
		char cur_char;
};
#endif
