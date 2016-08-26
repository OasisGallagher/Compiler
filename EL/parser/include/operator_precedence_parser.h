#pragma once
#include "parser.h"

class OperatorPrecedenceParser : public Parser {
public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner);
	virtual std::string ToString() const;
};
