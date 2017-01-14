#pragma once
#include "lr1.h"
#include "lr_impl.h"

class LR0;
class Grammar;
struct Condinate;

class LALR : public LRImpl {
public:
	LALR();
	~LALR();

public:
	virtual std::string ToString() const;

	virtual bool Parse(LRGotoTable& gotoTable, LRActionTable& actionTable);

	virtual void Setup(const LRSetupParameter& parameter);

protected:
	virtual bool CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable);

	bool CreateGotoTable(LRGotoTable &gotoTable);
	bool CreateActionTable(LRActionTable &actionTable);

private:
	bool IsNullable(const GrammarSymbol& symbol);

	void PropagateSymbols();
	bool PropagateSymbolsOnePass();
	bool PropagateFrom(const LR1Item &src);

	void CalculateForwardsAndPropagations(const GrammarSymbol& symbol);
	void AddForwardsAndPropagations(LR1Item& item, const LR1Itemset& itemset);

	bool CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);

	void CalculateLR1Itemset(LR1Itemset& answer);
	bool CalculateLR1ItemsetOnePass(LR1Itemset& answer);
	void CalculateLR1Items(LR1Itemset& answer, const LR1Item& item);

	void AddLR1Items(LR1Itemset& answer, const GrammarSymbol& forward, Grammar* g, int gi);

	bool ParseLRAction(LRActionTable & actionTable, const LR1Itemset& itemset, const LR1Item &item);

private:
	LRSetupParameter p_;

	LR1EdgeTable edges_;
	Propagations propagations_;
	LR1ItemsetContainer itemsets_;
};
