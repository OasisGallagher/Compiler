#pragma once
#include "lr1.h"
#include "lr_impl.h"
#include "parser.h"

struct Condinate;

class LR0;
class Grammar;
class Ambiguities;

class LALR : public LRImpl {
public:
	LALR();
	~LALR();

public:
	virtual std::string ToString() const;

	virtual bool Parse(LRActionTable& actionTable, LRGotoTable& gotoTable);

	virtual void Setup(Environment* env, FirstSetTable* firstSet);

protected:
	virtual bool CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable);

	bool CreateGotoTable(LRGotoTable &gotoTable);
	bool CreateActionTable(LRActionTable &actionTable);
	bool InsertActionTable(LRActionTable &actionTable, const LR1Itemset& src, const GrammarSymbol& symbol, const LRAction& action);

private:
	bool IsNullable(const GrammarSymbol& symbol);

	void PropagateSymbols();
	bool PropagateSymbolsOnePass();
	bool PropagateFrom(const LR1Item& src);

	LR1Item FindItem(int cpos, int dpos, LR1Itemset& dict);

	void CalculateForwardsAndPropagations();
	void AddForwardsAndPropagations(LR1Item& item, const LR1Itemset& itemset, LR1Itemset& dict, const GrammarSymbol& symbol);

	void CalculateLR1Itemset(LR1Itemset& answer, LR1Itemset& itemset);
	bool CalculateLR1ItemsetOnePass(LR1Itemset& answer, LR1Itemset& itemset);

	bool CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);

	void AddLR1Items(LR1Itemset &answer, const GrammarSymbol& lhs, const LR1Item &current, LR1Itemset& itemset);

	bool ParseLRAction(LRActionTable & actionTable, const LR1Itemset& itemset, const LR1Item &item);

private:
	Environment* env_;
	Ambiguities* ambiguities_;
	FirstSetTable* firstSets_;

	LR1Item tmp_;
	LR1EdgeTable edges_;

	int coreItemsCount_;
	Propagations propagations_;
	LR1ItemsetContainer itemsets_;
};
