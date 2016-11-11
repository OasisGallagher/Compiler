#pragma once
#include "lr0.h"
#include "parser.h"

enum LRActionType {
	LRActionError,
	LRActionShift,
	LRActionReduce,
	LRActionAccept,
};

struct LRAction {
	LRActionType actionType;
	int actionParameter;
};

class LRGotoTable : public matrix<int, GrammarSymbol, int> {
public:
	std::string ToString() const;
};

class LRActionTable : public matrix<int, GrammarSymbol, LRAction> {
public:
	std::string ToString() const;
};

class LR0;

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
	LR0* lr0_;

	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
