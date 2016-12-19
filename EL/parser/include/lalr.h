#pragma once
#include "lr1.h"
#include "lr_impl.h"

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
	bool MergeItemsets();

	void NormalizeStateNames(LR1ItemsetContainer& newItemsets);
	void RecalculateEdges(LR1EdgeTable& newEdges, LR1ItemsetContainer& newItemsets, ItemsetNameMap& nameMap);
	void RecalculateNewEdgeTarget(LR1EdgeTable& newEdges, const LR1Itemset& current, const GrammarSymbol& symbol, ItemsetNameMap& nameMap);

	bool CreateLR1Itemsets();
	bool CreateLR1ItemsetsOnePass();

	void CalculateLR1Itemset(LR1Itemset& answer);
	bool CalculateLR1ItemsetOnePass(LR1Itemset& answer);
	void CalculateClosureItems(LR1Itemset& answer, const GrammarSymbolSet& firsts, Grammar* g, int gi);
	void CalculateLR1ItemsetByLhs(LR1Itemset& answer, const LR1Item& item, const GrammarSymbol& lhs, const Condinate* cond);

	bool GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
	bool CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);

	bool IsNullable(const GrammarSymbol& symbol);
	bool ParseLRAction(LRActionTable & actionTable, const LR1ItemsetName& name, const LR1Item &item);

private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR1EdgeTable edges_;
	LR1ItemsetBuilder* builder_;
	LR1ItemsetContainer itemsets_;
};
