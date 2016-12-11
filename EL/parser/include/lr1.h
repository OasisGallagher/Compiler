#pragma once
#include <set>
#include <map>

#include "matrix.h"
#include "grammar.h"
#include "lr_impl.h"

struct LR1Item {
	int cpos, dpos;
	GrammarSymbol forward;

	SymbolVector* container;

	bool operator < (const LR1Item& other) const;
	bool operator == (const LR1Item& other) const;

	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1Itemset {
	typedef std::set<LR1Item> LR1Closure;
	IMPLEMENT_REFERENCE_COUNTABLE(LR1Itemset, LR1Closure);

public:
	LR1Itemset();

public:
	typedef LR1Closure::iterator iterator;
	typedef LR1Closure::const_iterator const_iterator;

public:
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	void clear();
	int size() const;
	bool empty() const;

	bool insert(const LR1Item& item);

public:
	bool operator < (const LR1Itemset& other) const;
	bool operator == (const LR1Itemset& other) const;

public:
	std::string ToString(const GrammarContainer& grammars) const;

private:
#pragma push_macro("new")
#undef new
	// LR1Itemset不可以通过new分配.
	void* operator new(size_t);
#pragma pop_macro("new")
};

class LR1ItemsetContainer : public std::set<LR1Itemset> {
public:
	void Merge();
	std::string ToString(const GrammarContainer& grammars) const;

private:
	void MergeItemset(LR1Itemset& answer, const LR1Itemset& itemset);
	void MergeForwardSymbols(SymbolVector* dest, LR1Itemset::const_iterator first, LR1Itemset::const_iterator last);
};

class LR1EdgeTable : public matrix<LR1Itemset, GrammarSymbol, LR1Itemset> {
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
	void MergeItemsets();

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
