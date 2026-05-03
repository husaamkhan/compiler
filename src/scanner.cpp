#include "scanner.h"
#include <cctype>
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
	else
	{
		return '\0';
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

void Scanner::handleHash()
{
	char ch = nextChar();

	switch (ch)
	{
		// Note to self for later, are these checks for \0 \n and ' ' really necessary?
		// How is the default case not sufficient for these?
		case '\0':
			stack.push(CategoryStatePair { ERROR, END_OF_FILE });
			break;

		case '\n':
			stack.push(CategoryStatePair { ERROR, WHITESPACE });
			break;

		case ' ':
			stack.push(CategoryStatePair { ERROR, WHITESPACE });
			break;

		case 't':
			stack = std::stack<CategoryStatePair>();
			stack.push(CategoryStatePair { START, NONE });
			stack.push(CategoryStatePair { ACCEPTING, BOOLEAN });
			break;

		case 'f':
			stack = std::stack<CategoryStatePair>();
			stack.push(CategoryStatePair { START, NONE });
			stack.push(CategoryStatePair { ACCEPTING, BOOLEAN });
			break;

		case '\\':
			stack.push(CategoryStatePair { NON_ACCEPTING, NONE });
			handleCharacter();
			break;
			
		default: // No match causes error state
			stack.push(CategoryStatePair {ERROR, NONE});
			break;
	}
}

void Scanner::handleCharacter()
{
	char ch = nextChar();

	if (ch == '\0')
	{
		stack.push(CategoryStatePair { ERROR, END_OF_FILE });
	}
	else
	{
		stack.push(CategoryStatePair { ACCEPTING, CHARACTER });
	}

	// Check if character name was provided
	// Valid cases are 'space' and 'newline'
	std::string valid_chars;
	if (ch == 's')
	{
		valid_chars = "pace";
	}
	else if (ch == 'n')
	{
		valid_chars = "ewline";
	}
	else
	{
		return;	
	}

	State state = NON_ACCEPTING;
	int next = 0;

	while (state != ERROR && next < valid_chars.size())
	{
		ch = nextChar();
		
		if (ch == valid_chars.at(next))
		{
			if (next+1 == valid_chars.size())
			{
				stack.push(CategoryStatePair { ACCEPTING, CHARACTER });
			}
			else
			{
				stack.push(CategoryStatePair { NON_ACCEPTING, NONE });
			}
		}
		else
		{
			stack.push(CategoryStatePair { ERROR, NONE });
		}

		state = stack.top().state;
		next++;
	}
}

void Scanner::handleIdentifier(char ch)
{
	if (std::isdigit(ch))
	{
		stack.push(CategoryStatePair { ERROR, NONE });
		return;
	}
	else
	{
		stack.push(CategoryStatePair { ACCEPTING, IDENTIFIER });	
	}

	State state = NON_ACCEPTING;

	while (state != ERROR)
	{
		char ch = nextChar();

		if (ch == ' ' || ch == '\n' || ch == '\0' || ch == '#' || ch == '|' || ch == '`' || ch == '\\' ||
				ch == '\'' || ch == ';' || ch == ',' || ch == '(' || ch == ')')
		{
			stack.push(CategoryStatePair { ERROR, NONE });
		}
		else
		{
			stack.push(CategoryStatePair {ACCEPTING, IDENTIFIER });
		}

		state = stack.top().state;
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
			case NONE:
			{
				std::string message = std::string("An error occured during scanning! Scanner couldn't recognize the following lexeme:") + token.lexeme;
				throw ScannerError{ message, "scan()" };
			}

			default:
				output->push(token);
		}
	}
}

Token Scanner::getNextToken()
{
	State state = NON_ACCEPTING;
	lexeme_start = cur_pos;

	char ch = nextChar();
	CategoryStatePair result;

	switch (ch)
	{
		case '\0':
			return Token { END_OF_FILE, "" };

		case ' ':
			return Token { WHITESPACE, "' '" };

		case '\n':
			return Token { WHITESPACE, "\\n" };

		case '#':
			handleHash();
			break;

		default:
			handleIdentifier(ch);
			break;
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

	std::string lexeme(double_buffer + lexeme_start, cur_pos - lexeme_start);

	if (state != ACCEPTING)
	{
		std::string message = std::string("An error occured during scanning! Scanner roll back couldn't reach an accepting state for lexeme: ") + lexeme;
		throw ScannerError{ message, "getNextToken()" };
	}

	return Token { category, lexeme };
}


