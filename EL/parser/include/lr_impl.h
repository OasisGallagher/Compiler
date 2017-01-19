#pragma once
#include "matrix.h"

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

struct Environment;

class LRGotoTable;
class LRActionTable;

class FirstSetTable;

class GrammarSymbol;
class GrammarContainer;
class GrammarSymbolSetTable;
class GrammarSymbolContainer;

class Conflicts : public matrix<int, GrammarSymbol, LRAction> {
};

class LRImpl {
public:
	virtual ~LRImpl() { }
	
public:
	virtual std::string ToString() const = 0;

	virtual bool Parse(LRActionTable& actionTable, LRGotoTable& gotoTable) = 0;
	virtual void Setup(Environment* env, FirstSetTable* firstSet) = 0;

protected:
	bool InsertActionTable(LRActionTable &actionTable, int src, const GrammarSymbol& symbol, const LRAction& action);

private:
	Conflicts conflicts_;
};
