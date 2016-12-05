#pragma once
#include <set>
#include <map>

#include "matrix.h"
#include "grammar.h"
#include "lr_impl.h"

struct LR1Item {
	int cpos, dpos;
	GrammarSymbol forward;

	bool operator < (const LR1Item& other) const;
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1Closure : public std::set<LR1Item> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1Itemsets : public std::set<LR1Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1ClosureContainer : public std::map<LR1Item, LR1Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1EdgeTable : public matrix<LR1Closure*, GrammarSymbol, LR1Closure*> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1 : public LRImpl {

public:
	LR1();
	~LR1();

public:
	virtual std::string ToString() const;

	virtual bool Parse(LRGotoTable& gotoTable, LRActionTable& actionTable);
	virtual void Setup(const LRSetupParameter& parameter);

protected:
	virtual bool CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable);

private:
	bool CreateLR1Itemsets();

	bool CreateLR1ItemsetsOnePass();

	LR1Closure* GetLR1Closure(const LR1Item& item);
	LR1Closure* CalculateLR1Closure(const LR1Item& item);
	bool CalculateLR1ClosureOnePass(LR1Closure* answer);

	LR1Closure* GetLR1EdgeTarget(LR1Closure* src, const GrammarSymbol& symbol);
	LR1Closure* CalculateLR1EdgeTarget(LR1Closure* src, const GrammarSymbol& symbol);

private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR1EdgeTable edges_;
	LR1ClosureContainer closures_;
	LR1Itemsets itemsets_;
};
