#pragma once
#include <set>
#include <map>

#include "matrix.h"
#include "grammar.h"
#include "lr_impl.h"

struct LR0Item {
	int cpos, dpos;

	bool operator < (const LR0Item& other) const;
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR0Closure : public std::set<LR0Item> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR0Itemsets : public std::set<LR0Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR0ClosureContainer : public std::map<LR0Item, LR0Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR0EdgeTable : public matrix<LR0Closure*, GrammarSymbol, LR0Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR0 : public LRImpl {

public:
	LR0();
	~LR0();

public:
	virtual std::string ToString() const;

	virtual bool Parse(LRGotoTable& gotoTable, LRActionTable& actionTable);
	virtual void Setup(const LRSetupParameter& parameter);

protected:
	virtual bool CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable);

private:
	bool CreateLR0Itemsets();

	bool CreateLR0ItemsetsOnePass();

	LR0Closure* GetLR0Closure(const LR0Item& item);
	LR0Closure* CalculateLR0Closure(const LR0Item& item);

	bool CalculateLR0ClosureOnePass(LR0Closure* answer);

	LR0Closure* GetLR0EdgeTarget(LR0Closure* src, const GrammarSymbol& symbol);
	LR0Closure* CalculateLR0EdgeTarget(LR0Closure* src, const GrammarSymbol& symbol);

private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR0EdgeTable edges_;
	LR0ClosureContainer closures_;
	LR0Itemsets itemsets_;
};
