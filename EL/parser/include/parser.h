#pragma once
#include <map>
#include "grammar_symbol.h"

class SyntaxTree;
class FileScanner;

class Parser {
public:
	Parser();
	virtual ~Parser();

public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner) = 0;
	virtual std::string ToString() const = 0;
};
