#include "scanner.h"
#include <stdexcept>
#include <string>

Scanner::Scanner() : input_stream(nullptr), output(nullptr)
{}

Scanner::~Scanner()
{}

void Scanner::init(std::istream* in, std::queue<Token>* out)
{
	input_stream = in;
	output = out;
	stack = std::stack<CategoryStatePair>();
	stack.push(CategoryStatePair { START, NONE });

	cur_pos = 0;
	fence = 0;
	lexeme_start = 0;
}

void Scanner::fillBuffer(int pos)
{
	input_stream->read(double_buffer + pos, BUFFER_LENGTH);
}

char Scanner::nextChar()
{
	char c = double_buffer[cur_pos];
	
	if (c != EOF) // EOF results in ERROR state, which ends the scanning loop and triggers rollback
	{
		cur_pos = (cur_pos + 1) % 2*BUFFER_LENGTH;

		// Fills next buffer if current position has crossed into it and updates fence
		if ((cur_pos % BUFFER_LENGTH) == 0)
		{
			fillBuffer(cur_pos);
			fence = (cur_pos + BUFFER_LENGTH) % 2*BUFFER_LENGTH;
		}
	}

	return c;
}

void Scanner::rollBack()
{
	if (cur_pos == fence)
	{
		std::string message = 	std::string("Error during rollback! Attempted to rollback into invalid context! Fence: ") +
					std::to_string(fence) + "\n" +
					"Current position: " + std::to_string(cur_pos);
		throw ScannerError{ message, "rollBack()" };
	}

	cur_pos = (cur_pos - 1)	% 2*BUFFER_LENGTH;
}

CategoryStatePair Scanner::handleHash()
{
	char cur = nextChar();
	switch (cur)
	{
		case 't':
			return CategoryStatePair {ACCEPTING, BOOLEAN};
		case 'f':
			return CategoryStatePair {ACCEPTING, BOOLEAN};
		default: // No match causes error state
			return CategoryStatePair {ERROR, NONE};
	}
}

void Scanner::scan()
{
	input_stream->read(double_buffer, BUFFER_LENGTH);
	fillBuffer(0);

	Token token;

	while (token.category != END_OF_FILE)
	{
		token = getNextToken();

		if (token.category == NONE)
		{
			std::string message = std::string("An error occured during scanning! Couldn't get token for lexeme:") + token.lexeme;
			throw ScannerError{ message, "scan()" };
		}

		output->push(token);
	}
}

Token Scanner::getNextToken()
{
	State state = NON_ACCEPTING;
	Category category = NONE;

	// Scan loop finds the longest possible match for a token
	while (state != ERROR)
	{
		char ch = nextChar();
		lexeme_start = cur_pos;

		CategoryStatePair result;

		switch (ch)
		{
			case '#':
				result = handleHash();
				break;
		}

		state = result.state;

		if (state == ACCEPTING)
		{
			stack = std::stack<CategoryStatePair>();
			stack.push(CategoryStatePair { START, NONE });
		}

		stack.push(CategoryStatePair { state, result.category });
	}

	// Roll back to truncate lexeme and rewind cur_pos back to the end of the resulting token
	while(state != ACCEPTING && state != START)
	{
		state = stack.top().state;
		stack.pop();

		if (state != START)
		{
			rollBack();
		}
	}


	std::string lexeme(double_buffer + lexeme_start, cur_pos - lexeme_start);

	if (state != ACCEPTING)
	{
		std::string message = 	std::string("An error occured during scanning! Scanner roll back couldn't reach an accepting state for lexeme: .") +
					lexeme;
		throw ScannerError{ message, "getNextToken()" };
	}

	return Token { category, lexeme };
}


