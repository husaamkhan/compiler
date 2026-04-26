#include "scanner.h"

Scanner::Scanner() : input_stream(nullptr)
{}

Scanner::~Scanner()
{}

void Scanner::init(std::istream* stream)
{
	input_stream = stream;
	int cur_pos = 0;
}

void Scanner::fillBuffer(int pos)
{
	input_stream->read(double_buffer + pos, BUFFER_LENGTH);
}

char Scanner::nextChar()
{
	char c = this->double_buffer[this->cur_pos];
	
	if (c != EOF)
	{
		this->cur_pos = (this->cur_pos + 1) % 2*BUFFER_LENGTH;

		// Fills next buffer if current position has crossed into it and updates fence
		if ((cur_pos % BUFFER_LENGTH) == 0)
		{
			this->fillBuffer(cur_pos);
			this->fence = (cur_pos + BUFFER_LENGTH) % 2*BUFFER_LENGTH;
		}
	}

	return c;
}

void Scanner::scan()
{
	input_stream->read(double_buffer, BUFFER_LENGTH);
	this->fillBuffer(0);

	std::string lexeme = "";
	Token token = INCOMPLETE;

	State state = NON_ACCEPTING; 

	while (state != ERROR)
	{
		char cur = nextChar();
	}
}

