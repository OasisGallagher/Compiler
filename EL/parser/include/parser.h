#pragma once
#include <map>
#include "grammar_symbol.h"

class SyntaxTree;
class FileScanner;

class BuildinSymbolContainer : public std::map < std::string, GrammarSymbol > {
public:
	BuildinSymbolContainer();

public:
	std::string ToString() const;
};

class Parser {
public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner) = 0;
	virtual std::string ToString() const = 0;
};
