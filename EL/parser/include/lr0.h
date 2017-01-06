#pragma once

#include "lr1.h"
#include "grammar.h"
#include "lr_impl.h"

class LR0 { 

public:
	LR0();
	~LR0();

public:
	std::string ToString() const;
	void Setup(const LRSetupParameter& parameter);
	bool CreateLR0Itemsets(LR1ItemsetContainer& itemsets, LR1EdgeTable& edges);

private:
	void CalculateClosure(LR1Itemset& answer);
	bool IsNullable(const GrammarSymbol& symbol);
	void AddLR1Items(LR1Itemset& answer, const GrammarSymbol& lhs);
	bool CalculateClosureOnePass(LR1Itemset& answer);
	bool CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
	bool GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol);
	bool CreateLR1ItemsetsOnePass();

private:
	LRSetupParameter p_;

	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer_;

	LR1EdgeTable edges_;
	LR1ItemsetContainer itemsets_;
};
