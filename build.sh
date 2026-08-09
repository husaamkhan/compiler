#!/bin/bash

compiler=gcc
flags="-Wall -Wextra -Werror"
compiler_definitions=""
includes="-Iinclude"
sources="src/lexer.c"
target="lexer_cli"
test=0

# Configure build based on input parameter
if [[ $# -eq 0 ]]; then
	echo "Default target selected."
	sources="$sources tools/lexer_cli.c"
else
	if [[ "$1" == "debug" ]]; then
		echo "Debug target selected."
		compiler_definitions="-DDEBUG"
		sources="$sources tools/lexer_cli.c"
	elif [[ "$1" == "test" ]]; then
		echo "Test target selected."
		compiler_definitions="-DDEBUG"
		includes="$includes -Iexternal/unity/src"
		sources="$sources external/unity/src/unity.c test/test_lexer.c"
		target="test_lexer"
		test=1
	else
		echo "Invalid target selected. "
		echo "Usage: build.sh [optional target]"
		echo "Targets: debug | test"
		exit
	fi
fi

# Build target
$compiler $flags $compiler_definitions $includes $sources -o $target

if [[ $? -eq 0 ]]; then
	echo "Successfully built $target"
else
	exit
fi

if [[ $test -eq 1 ]]; then
	echo "Running $target"
	./$target
fi
