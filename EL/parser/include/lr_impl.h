#pragma once
#include "matrix.h"

class FirstSetTable;

class GrammarSymbol;
class GrammarContainer;
class GrammarSymbolSetTable;
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

	bool operator == (const LRAction& other) const;
	bool operator != (const LRAction& other) const;

	std::string ToString() const;
};

class LRGotoTable : public matrix<int, GrammarSymbol, int> {
public:
	std::string ToString() const;
};

class LRActionTable : public matrix<int, GrammarSymbol, LRAction> {
public:
	std::string ToString() const;
};

struct LRSetupParameter {
	GrammarContainer* grammars;
	GrammarSymbolContainer* terminalSymbols;
	GrammarSymbolContainer* nonterminalSymbols;

	FirstSetTable* firstSetContainer;
	GrammarSymbolSetTable* followSetContainer;
};

class LRImpl {
public:
	virtual ~LRImpl() { }
	
public:
	virtual std::string ToString() const = 0;

	virtual bool Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) = 0;
	virtual void Setup(const LRSetupParameter& parameter) = 0;

protected:
	bool InsertActionTable(LRActionTable &actionTable, int src, const GrammarSymbol& symbol, const LRAction& action);
};
