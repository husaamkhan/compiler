#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

void printUsage()
{
	cerr	<< "Usage:\n"
		<< "./scanner_cli              -    Enter text interactively\n"
		<< "./scanner_cli -f <file>    -    Read input from file\n";
}

string readFromShell()
{
	string input;
	
	// Read user's input in shell until user types Ctrl+D
	string str = "";
	while(std::getline(cin, str))
	{
		input += str + "\n";
	}

	return input;
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
	if (argc > 3)
	{
		printUsage();
	}

	string input;
	if (argc == 1) // If user only enters ./scanner_cli, read their input from shell until they enter Ctrl+D
	{
		input = readFromShell();
	}
	else if (argc == 3 && strncmp(argv[1], "-f", 2) == 0) // User must enter ./scanner_cli -f <file> to pass input from file
	{
		input = readFromFile(argv[2]);
	}
	else // Any other input is invalid, print usage instructions
	{
		printUsage();
	}

	cout << input << endl;
}

