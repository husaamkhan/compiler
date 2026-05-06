#include <iostream>
#include <fstream>
#include "scanner.h"

using namespace std;

void printUsage()
{
	cerr	<< "Usage:\n"
		<< "./scanner_cli              	-    Enter text interactively\n"
		<< "./scanner_cli <file>    	-    Read input from file\n";
}

void printTokens(queue<Token> &tokens)
{
	while(!tokens.empty())
	{
		string category = "";
		switch(tokens.front().category)
		{
			case BOOLEAN:
				category = "BOOLEAN";
				break;

			case CHARACTER:
				category = "CHARACTER";
				break;

			case IDENTIFIER:
				category = "IDENTIFIER";
				break;

			case STRING:
				category = "STRING";
				break;

			case WHITESPACE:
				category = "WHITESPACE";
				break;

			case END_OF_FILE:
				category = "END_OF_FILE";
				break;

			default:
				category = "NONE";
				break;
		}

		cout << string("[") + tokens.front().lexeme + ": " + category + "]"<< endl;
		tokens.pop();
	}
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		printUsage();
	}

	Scanner* scanner = new Scanner();

	ifstream file;
	if (argc == 1)
	{
		scanner->init(&cin);
	}
	else
	{
		file = ifstream(argv[1]);
		if (!file.is_open())
		{
			cerr<< "Could not open file: " << argv[2] << endl;
		}

		scanner->init(&file);
	}

	try
	{
		queue<Token> token_queue;
		scanner->scan(&token_queue);
		printTokens(token_queue);
	}
	catch (exception& e)
	{
		cerr << string("Error: ") + e.what() << endl;
	}
}

