#pragma once
#include "matrix.h"

class GrammarSymbol;
class GrammarContainer;
class GrammarSymbolContainer;

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

class LRImpl {
public:
	virtual ~LRImpl() { }

public:
	virtual std::string ToString() const = 0;

	virtual bool Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) = 0;
	virtual void Setup(GrammarContainer* grammars, 
		GrammarSymbolContainer* terminalSymbols, GrammarSymbolContainer* nonterminalSymbols) = 0;
};
