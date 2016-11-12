#pragma once
#include "parser.h"

class LRTable;

class LRParser : public Parser {
public:
	LRParser();
	~LRParser();

public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner);
	virtual std::string ToString() const;

protected:
	virtual bool ParseGrammars();
	virtual void Clear();

private:
	GrammarSymbol ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner);
	
private:
	LRTable* lrTable_;
};
