#pragma once
#include "lr1.h"
#include "lr_impl.h"

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
	void MergeItemsets();
	void MergeNewItemset(LR1Itemset &newSet, LR1ItemsetContainer::const_iterator first, LR1ItemsetContainer::const_iterator last);
	void FindMergeTarget(LR1Itemset& answer, const std::string& name, const LR1ItemsetContainer& newItemsets);

	void RecalculateEdges(LR1EdgeTable& newEdges, const LR1ItemsetContainer& newItemsets);
	void RecalculateNewEdgeTarget(LR1Itemset& answer, const LR1Itemset& current, const GrammarSymbol& symbol, const LR1ItemsetContainer& newItemsets);

	bool CreateLR1Itemsets();
	bool CreateLR1ItemsetsOnePass();

	void CalculateLR1Itemset(LR1Itemset& answer);
	bool CalculateLR1ItemsetOnePass(LR1Itemset& answer);

	bool GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
	bool CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);

	int StateName2Integer(const std::string& name);
private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR1EdgeTable edges_;
	LR1ItemsetContainer itemsets_;
};
