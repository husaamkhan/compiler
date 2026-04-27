#include <cstdio>
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string.h>
#include "scanner.h"

using namespace std;

void printUsage()
{
	cerr	<< "Usage:\n"
		<< "./scanner_cli              	-    Enter text interactively\n"
		<< "./scanner_cli <file>    	-    Read input from file\n";
}

string readFromFile(string path)
{
	ifstream file(path);

	if (!file.is_open())
	{
		cerr << "Could not open file: " << path << endl;
	}

	// Read contents of file to string
	string contents((
				std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

	return contents;
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		printUsage();
	}

	queue<Token> token_queue;
	Scanner* scanner = new Scanner();

	ifstream file;
	if (argc == 1)
	{
		scanner->init(&cin, &token_queue);
	}
	else
	{
		file = ifstream(argv[1]);
		if (!file.is_open())
		{
			cerr<< "Could not open file: " << argv[2] << endl;
		}

		scanner->init(&file, &token_queue);
	}

	try
	{
		scanner->scan();
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}

