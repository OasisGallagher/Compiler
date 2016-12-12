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

private:
	void Merge();
	void MergeItemsets(const LR1ItemsetVector& itemsets);
	void MergeNewItemset(LR1Itemset &newSet, LR1ItemsetVector::const_iterator first, LR1ItemsetVector::const_iterator last);

	void RecalculateEdges(LR1EdgeTable& newEdges, const LR1ItemsetContainer& newItemsets);
	void RecalculateNewEdgeTarget(LR1Itemset& answer, const LR1Itemset& current, const GrammarSymbol& symbol, const LR1ItemsetContainer& newItemsets);

	void MergeForwardSymbols(LR1Itemset& answer, const LR1Itemset& itemset);

	bool CreateLR1Itemsets();
	bool CreateLR1ItemsetsOnePass();

	void CalculateLR1Itemset(LR1Itemset& answer);
	bool CalculateLR1ItemsetOnePass(LR1Itemset& answer);

	bool GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
	void CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR1EdgeTable edges_;
	LR1ItemsetContainer itemsets_;
};
