#include <iostream>
#include "scanner.h"

using namespace std;

int main() {
	cout << "Hello world!" << endl;

	Scanner* scanner = new Scanner();
	scanner->scan("hello");
}
