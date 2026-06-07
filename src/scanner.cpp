#include "scanner.h"
#include <cctype>
#include <cstring>
#include <string>

void Scanner::init(std::istream *in)
{
	in->seekg(0, std::ios::end);
	std::streamsize size = in->tellg();
	in->seekg(0, std::ios::beg);

	file_contents = std::vector<char>(static_cast<size_t>(size));

	if (!in->read(file_contents.data(), size))
	{
		std::string message = std::string("Error: Couldn't read file");
		throw ScannerError { message, "init()" };
	}

	cur_pos 	= 0;
	lexeme_start 	= 0;
}

char Scanner::nextChar()
{
	cur_pos++;
	if (cur_pos >= file_contents.size())
	{
		return '\0';
	}

	return file_contents[cur_pos];
}

// Checks the next character after a # is read to handle booleans, characters, and numbers 
std::string Scanner::lookAhead(int n)
{
	if ((cur_pos + n) > file_contents.size())
	{
		n = file_contents.size() - cur_pos;
	}
	
	return std::string(file_contents.begin() + cur_pos, file_contents.begin() + cur_pos + n);
}

// A # can lead to multiple categories. This function checks the next character after a #
// is read to handle booleans, characters, and numbers
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
	// Look ahead to check for \\space or \\newline
	if (lookAhead(5).compare("space") == 0)
	{
		cur_pos += 5;
	}
	else if (lookAhead(7).compare("newline") == 0)
	{
		cur_pos += 7;
	}
	else
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
	}

	last_accepting_pos = cur_pos;
	category = CHARACTER;
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

	// TODO: We break from the loop on \0, doesn't that mean that these lines
	// can be reached with an unclosed string?
	category = STRING;
	last_accepting_pos = cur_pos;
}

// This function is called when expecting the prefix, which happens when either the radix
// or the exactness has been recognized
void Scanner::handleNumber()
{
	char ch = nextChar();

	// Optional second prefix component (#e/#i/#b/#o/#d/#x)
	if (ch == '#')
	{
		ch = nextChar();
		if (ch != 'i' && ch != 'e' && ch != 'b' && ch != 'o' && ch != 'd' && ch != 'x')
			return;
		ch = nextChar();
	}

	// Scan <real R> = <sign> <ureal R> using a state machine
	// States reflect what characters are valid after each symbol consumed
	// Returns the next unprocessed character
	enum ScanState { SIGN, DIGITS, HASHES, DOT, DOT_DIGITS, SLASH, SLASH_DIGITS, EXP_MARKER, EXP_SIGN, EXP_DIGITS };

	auto scanReal = [&]() -> char
	{
		ScanState state = SIGN;

		while (true)
		{
			ScanState next_state = state;

			switch (state)
			{
				case SIGN:
					if (ch == '+' || ch == '-')         { next_state = DIGITS; break; }
					// fall through for unsigned real
				case DIGITS:
					if (std::isdigit(ch))               { next_state = DIGITS; category = NUMBER; last_accepting_pos = cur_pos; break; }
					if (ch == '#')                      { next_state = HASHES; category = NUMBER; last_accepting_pos = cur_pos; break; }
					if (ch == '.')                      { next_state = DOT; break; }
					if (ch == '/')                      { next_state = SLASH; break; }
					if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
					                                    { next_state = EXP_MARKER; break; }
					return ch;

				case HASHES:
					if (ch == '#')                      { break; }
					if (ch == '.')                      { next_state = DOT; break; }
					if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
					                                    { next_state = EXP_MARKER; break; }
					return ch;

				case DOT:
				case DOT_DIGITS:
					if (std::isdigit(ch))               { next_state = DOT_DIGITS; category = NUMBER; last_accepting_pos = cur_pos; break; }
					if (ch == '#')                      { next_state = HASHES; category = NUMBER; last_accepting_pos = cur_pos; break; }
					if (ch == 'e' || ch == 's' || ch == 'f' || ch == 'd' || ch == 'l')
					                                    { next_state = EXP_MARKER; break; }
					return ch;

				case SLASH:
				case SLASH_DIGITS:
					if (std::isdigit(ch))               { next_state = SLASH_DIGITS; category = NUMBER; last_accepting_pos = cur_pos; break; }
					if (ch == '#')                      { next_state = HASHES; category = NUMBER; last_accepting_pos = cur_pos; break; }
					return ch;

				case EXP_MARKER:
				case EXP_SIGN:
					if (ch == '+' || ch == '-')         { next_state = EXP_SIGN; break; }
					// fall through
				case EXP_DIGITS:
					if (std::isdigit(ch))               { next_state = EXP_DIGITS; category = NUMBER; last_accepting_pos = cur_pos; break; }
					return ch;
			}

			state = next_state;
			ch = nextChar();
		}
	};

	ch = scanReal();

	if (ch == '@')
	{
		// Polar complex: <real R> @ <real R>
		ch = nextChar();
		ch = scanReal();
	}
	else if (ch == '+' || ch == '-')
	{
		// Rectangular complex: <real R> +/- <ureal R> i  or  <real R> +/- i
		ch = nextChar();
		ch = scanReal();
	}

	// In R5RS, 'i' can only appear as the last character of a number
	if (ch == 'i')
	{
		category = NUMBER;
		last_accepting_pos = cur_pos;
	}
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


