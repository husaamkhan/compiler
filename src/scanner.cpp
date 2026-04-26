#include "scanner.h"
#include <stdexcept>

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
	if (fence == BUFFER_LENGTH)
	{
		if (last_accepting_pos <= fence)
		{
			throw std::runtime_error(std::string("Error during scanner rollback! Attempted to rollback into invalid context.\n") +
						"Fence: " + std::to_string(fence) + "\n" +
						"Last accepting position: " + std::to_string(last_accepting_pos));
		}
	}
	else if (fence == 0)
	{
		if (last_accepting_pos <= 2*BUFFER_LENGTH && last_accepting_pos > BUFFER_LENGTH)
		{
			throw std::runtime_error(
					std::string("Error during scanner rollback! Attempted to rollback into invalid context.\n") +
					"Fence: " + std::to_string(fence) + "\n" +
					"Last accepting position: " + std::to_string(last_accepting_pos));
		}
	}
	else
	{
		throw std::runtime_error("Error during scanner rollback! Invalid fence location:" + std::to_string(fence));
	}

	// Truncate lexeme to last accepting state
	lexeme.resize(cur_pos - last_accepting_pos);
	cur_pos = last_accepting_pos;
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
	}
}

