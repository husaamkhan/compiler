#include "scanner.h"
#include <cctype>
#include <string>

void Scanner::init(std::istream *in)
{
	in->seekg(0, std::ios::end);
	std::streamsize size = in->tellg();
	in->seekg(0, std::ios::beg);

	file_contents = std::vector<char>(static_cast<size_t>(size));

	if (!in->read(this->file_contents.data(), size))
	{
		std::string message = std::string("Error: Couldn't read file");
		throw ScannerError { message, "init()" };
	}

	cur_pos 	= 0;
	lexeme_start 	= 0;
}

// TODO: Update to read from full file contents rather than double buffer
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

// Checks the next character after a # is read to handle booleans, characters, and numbers 
void Scanner::handleHash()
{
	char ch = nextChar();

	switch (ch)
	{
		case 't':
			category = BOOLEAN;
			last_accepting_pos = cur_pos;
			break;

		case 'f':
			category = BOOLEAN;
			last_accepting_pos = cur_pos;
			break;

		case '\\':
			handleCharacter();
			break;

		case 'i': // -+ Handle exactness
		case 'e': // -+
		case 'b': // -+ Handle radices
		case 'o': //  |
		case 'd': //  |
		case 'x': // -+
			// In the case where a # is read, The prefix of a number can start with either
			// the exactness or the radix. The possible options for these are:
			// Exactness: #i, #e | Radix: #b, #o, #d, #x
			handleNumber();
			break;
			
		default:
			break;
	}
}

void Scanner::handleCharacter()
{
	char ch = nextChar();

	if (ch == '\0')
	{
		// TODO: Is it possible that the program gets stuck in a loop here? It could
		// be possible that the scanner reaches this point, then jumps back to the
		// last accepting position, only to scan all the way back to this point
		// resulting in a loop

		return; // Unexpected end of file
	}
	else
	{
		last_accepting_pos = cur_pos;
		category = CHARACTER;
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

	bool valid_char = true;
	for (int i = 0; i < valid_chars.size(); i++)
	{
		ch = nextChar();

		if (ch != valid_chars.at(i))
		{
			valid_char = false;
			break;
		}
	}

	if (valid_char)
	{
		category = CHARACTER;
		last_accepting_pos = cur_pos;
	}
}

void Scanner::handleIdentifier(char ch)
{
	// Identifiers are not allowed to start with a digit
	if (std::isdigit(ch))
	{
		return; // TODO: Could this cause the program to be stuck in a loop?
	}
	else
	{
		category = IDENTIFIER;
		last_accepting_pos = cur_pos;
	}

	while (true)
	{
		char ch = nextChar();

		if (ch == ' ' || ch == '\n' || ch == '\0' || ch == '#' || ch == '|' || ch == '`' || ch == '\\' ||
				ch == '\'' || ch == ';' || ch == ',' || ch == '(' || ch == ')')
		{
			break;
		}
		else
		{
			category = IDENTIFIER;
			last_accepting_pos = cur_pos;
		}
	}
}

void Scanner::handleString()
{
	char ch = nextChar();
	while (ch != '"')
	{
		char ch = nextChar();

		if (ch == '\0')
		{
			break;
		}

		// Strings can have any character except for " and \, unless they form \" or \\
		// To handle this, the scanner needs to look ahead to see what the next character is
		if (ch == '\\')
		{
			char next_ch = nextChar();
			if (next_ch != '"' && next_ch != '\\')
			{
				return;
			}
		}
	}

	category = STRING;
	last_accepting_pos = cur_pos;
}

// TODO: Complete, and update to remove state machine logic
void Scanner::handleNumber()
{
	// This function is called when expecting the prefix, which happens when either the radix
	// or the exactness has been recognized.
	
	char ch = nextChar();

	// The remaining portion of the prefix can be provided, or left empty
	if (ch == '#')
	{
		ch = nextChar();
		if (ch == 'i' || ch == 'e' || ch == 'b' || ch == 'o' || ch == 'd' || ch == 'x')
		{
			stack.push(CategoryStatePair { NON_ACCEPTING, NONE });
		}
		else
		{
			stack.push(CategoryStatePair { ERROR, NONE });	
			return;
		}
	}

	// The remaining portion of the number is the complex portion, which can be any of the following:
	// <real R> | <real R> @ <real R> | <real R> + <ureal R> i | <real R> - <ureal R> i |
	// <real R> + i | <real R> - i | + <ureal R> i | - <ureal R> i | + i | - i
	ch = nextChar();

	// Complex portion can start with either signed or unsigned real number, or +/- i
	if (ch == '+' || ch == '-')
	{
		stack.push(CategoryStatePair { NON_ACCEPTING, NONE });

		ch = nextChar();
		if (ch == 'i')
		{
			stack.push(CategoryStatePair { ACCEPTING, NUMBER });
			return; // R5RS allows the imaginary portion to only be at the end of the number
		}
	}

	// TODO: What if this is a decimal number?
	
	// <real R> is defined as <sign> <ureal R>
	// <ureal R> -> <uinteger R> | <uinteger R> / <uinteger R> | <decimal R>
	// Scan for <ureal R>
	while (std::isdigit(ch))
	{
		stack.push(CategoryStatePair { ACCEPTING, NUMBER });
		ch = nextChar();
	}

	while (ch == '#')
	{
		stack.push(CategoryStatePair { ACCEPTING, NUMBER });
		ch = nextChar();
	}

	if (ch == '/')
	{
		stack.push(CategoryStatePair { NON_ACCEPTING, NONE });
		ch = nextChar();
		while (std::isdigit(ch))
		{
			stack.push(CategoryStatePair { ACCEPTING, NUMBER });
			ch = nextChar();
		}
	}

	// TODO: complete <ureal R> recognition, only <uinteger R> and <uinteger R> / <uinteger R> have
	// been implemented, still have to add <decimal R>
	
	// TODO: Add check for a +/- i at the end of the number
}

void Scanner::scan(std::queue<Token>* output)
{
	Token token;

	while (token.category != END_OF_FILE)
	{
		token = getNextToken();

		switch (token.category)
		{
			case NONE:
			{
				std::string message = std::string("An error occured during scanning! Scanner couldn't recognize the following lexeme:") + token.lexeme;
				// TODO: we need to instead have a way where we replace this with a collection
				// of errors that have occured. This will allow the compiler to scan the whole
				// document so that the user can see all of the errors that occured.
				throw ScannerError{ message, "scan()" };
			}

			default:
				output->push(token);
		}
	}
}

Token Scanner::getNextToken()
{
	category = NONE;
	lexeme_start = cur_pos;

	char ch = nextChar();
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

		case '"':
			handleString();
			break;

		default:
			handleIdentifier(ch);
			break;
	}

	// Jump back to last accepting token
	cur_pos = last_accepting_pos;
	std::string lexeme(file_contents.data() + lexeme_start, cur_pos - lexeme_start);

	if (category == NONE)
	{
		std::string message = std::string("Scanner couldn't categorize lexeme: ") + lexeme;

		// TODO: we need to instead have a way where we replace this with a collection
		// of errors that have occured. This will allow the compiler to scan the whole
		// document so that the user can see all of the errors that occured.
		throw ScannerError{ message, "getNextToken()" };
	}

	return Token { category, lexeme };
}


