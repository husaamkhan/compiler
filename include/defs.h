#include <stdexcept>
#include <string>

enum Category
{
	NONE,
	IDENTIFIER,
	BOOLEAN,
	NUMBER,
	CHARACTER,
	STRING,
	L_PARENTHESIS,
	R_PARENTHESIS,
	VECTOR_START,
	QUOTE,
	QUASIQUOTE,
	UNQUOTE,
	UNQUOTE_SPLICE,
	DOT,
	NEWLINE,
	END_OF_FILE
};

struct Token
{
	Category category = NONE;
	std::string lexeme = "";
};


struct ScannerError : public std::runtime_error
{
	std::string function;

	ScannerError(const std::string& message, const std::string& function)
		: std::runtime_error(message), function(function) {}

	const char* what() const noexcept override
	{
		static std::string msg;
		msg = "ScannerError in function " + function + ": " + std::runtime_error::what();
		return msg.c_str();
	}
};

