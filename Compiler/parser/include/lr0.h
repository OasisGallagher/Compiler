#pragma once

#include "lr1.h"
#include "grammar.h"
#include "lr_impl.h"

struct Environment;

class LR0 {
public:
	LR0();
	~LR0();

public:
	std::string ToString() const;
	void Setup(Environment* env, FirstSetTable* firstSets);
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
	Environment* env_;
	FirstSetTable* firstSets_;

	LR1EdgeTable edges_;
	LR1ItemsetContainer itemsets_;
};
