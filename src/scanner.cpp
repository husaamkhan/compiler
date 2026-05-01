#include "scanner.h"
#include <string>

Scanner::Scanner() : input_stream(nullptr)
{}

Scanner::~Scanner()
{}

void Scanner::init(std::istream* in)
{
	input_stream = in;
	stack = std::stack<CategoryStatePair>();
	stack.push(CategoryStatePair { START, NONE });

	buffer_size 	= 0;
	cur_pos 	= 0;
	fence 		= 0;
	lexeme_start 	= 0;
}

void Scanner::fillBuffer(int pos)
{
	input_stream->read(double_buffer + pos, BUFFER_LENGTH);
	buffer_size = input_stream->gcount();
}


char Scanner::nextChar()
{
	char c = double_buffer[cur_pos];
	
	if (!(cur_pos > (fence + buffer_size)))
	{
		cur_pos = (cur_pos + 1) % (2*BUFFER_LENGTH);

		// Fills next buffer if current position has crossed into it and updates fence
		if ((cur_pos % BUFFER_LENGTH) == 0)
		{
			fillBuffer(cur_pos);
			fence = (cur_pos + BUFFER_LENGTH) % (2*BUFFER_LENGTH);
		}
	}

	return c;
}

void Scanner::rollBack()
{
	if (cur_pos == fence)
	{
		std::string message = 	std::string("Error during rollback! Attempted to rollback into invalid context! Fence: ") +
					std::to_string(fence) + " " + "Current position: " + std::to_string(cur_pos);
		throw ScannerError{ message, "rollBack()" };
	}

	cur_pos = (cur_pos - 1)	% (2*BUFFER_LENGTH);
}

CategoryStatePair Scanner::handleHash()
{
	char ch = nextChar();
	switch (ch)
	{
		case 't':
			return CategoryStatePair {ACCEPTING, BOOLEAN};
		case 'f':
			return CategoryStatePair {ACCEPTING, BOOLEAN};
		default: // No match causes error state
			return CategoryStatePair {ERROR, NONE};
	}
}

void Scanner::scan(std::queue<Token>* output)
{
	fillBuffer(0);

	Token token;

	while (token.category != END_OF_FILE)
	{
		token = getNextToken();

		switch (token.category)
		{
			case NEWLINE:
				continue;

			case NONE:
			{
				std::string message = std::string("An error occured during scanning! Couldn't get token for lexeme:") + token.lexeme;
				throw ScannerError{ message, "scan()" };
			}

			default:
				output->push(token);
		}
	}
}

Token Scanner::getNextToken()
{
	State state 	= NON_ACCEPTING;
	lexeme_start 	= cur_pos;
	bool eof 	= false;

	// Scan loop finds the longest possible match for a token
	while (state != ERROR)
	{
		if (cur_pos+1 > (fence + buffer_size))
		{
			state = ERROR;
			stack.push(CategoryStatePair { ERROR, END_OF_FILE });
			
			eof = true;
			continue;
		}

		char ch = nextChar();

		CategoryStatePair result;

		switch (ch)
		{
			case '\n':
				result = CategoryStatePair { NON_ACCEPTING, NEWLINE };
				break;	

			case '#':
				result = handleHash();
				break;

			default:
				state = ERROR;
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

	Category category = NONE;

	// Roll back to truncate lexeme and rewind cur_pos to the end of the previous identified token
	while(state != ACCEPTING && state != START)
	{
		state = stack.top().state;
		category = stack.top().category;

		stack.pop();

		if (state != ACCEPTING && state != START)
		{
			rollBack();
		}
	}

	std::string lexeme(double_buffer + lexeme_start, cur_pos - lexeme_start + 1);

	cur_pos++;

	if (state != ACCEPTING && !eof)
	{
		std::string message = std::string("An error occured during scanning! Scanner roll back couldn't reach an accepting state for lexeme: ") + lexeme;
		throw ScannerError{ message, "getNextToken()" };
	}

	if (eof && state != ACCEPTING)
	{
		return Token { END_OF_FILE, "" };
	}

	return Token { category, lexeme };
}


