#include "scanner.h"
#include <stdexcept>
#include <string>

Scanner::Scanner() : input_stream(nullptr)
{}

Scanner::~Scanner()
{}

void Scanner::init(std::istream* stream)
{
	input_stream = stream;
	cur_pos = 0;
	fence = 0;
	lexeme = "";
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
		throw std::runtime_error(
				std::string("Error during rollback! Attempted to rollback into invalid context!") + "\n" +
				"Fence: " + std::to_string(fence) + "\n" +
				"Current position: " + std::to_string(cur_pos));
	}

	cur_pos = (cur_pos - 1)	% 2*BUFFER_LENGTH;
}

void Scanner::scan()
{
	input_stream->read(double_buffer, BUFFER_LENGTH);
	fillBuffer(0);

	std::string lexeme = "";
	Token token = INCOMPLETE;

	State state = NON_ACCEPTING; 

	while (state != ERROR)
	{
		char cur = nextChar();
		lexeme += cur;
		// get state
		// push state onto stack
	}
}

