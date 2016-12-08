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
	bool operator == (const LR1Item& other) const;

	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1ClosureImpl : public RefCountable, public std::set<LR1Item> {
	friend class LR1Closure;
};

class LR1Closure {
public:
	LR1Closure();
	LR1Closure(const LR1Closure& other);

	~LR1Closure();

public:
	typedef LR1ClosureImpl::iterator iterator;
	typedef LR1ClosureImpl::const_iterator const_iterator;

public:
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	void clear();
	bool empty() const;
	bool insert(const LR1Item& item);

public:
	LR1Closure& LR1Closure::operator = (const LR1Closure& other);

public:
	bool operator < (const LR1Closure& other) const;
	bool operator == (const LR1Closure& other) const;

public:
	std::string ToString(const GrammarContainer& grammars) const;

private:
	LR1ClosureImpl* impl_;
};

class LR1Itemsets : public std::set<LR1Closure> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1ClosureContainer : public std::map<LR1Item, LR1Closure> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1EdgeTable : public matrix<LR1Closure, GrammarSymbol, LR1Closure> {
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

	void CalculateLR1Closure(LR1Closure& answer);
	bool CalculateLR1ClosureOnePass(LR1Closure& answer);

	bool GetLR1EdgeTarget(LR1Closure& answer, const LR1Closure& src, const GrammarSymbol& symbol);
	void CalculateLR1EdgeTarget(LR1Closure& answer, const LR1Closure& src, const GrammarSymbol& symbol);

private:
	GrammarContainer* grammars_;
	GrammarSymbolContainer* terminalSymbols_;
	GrammarSymbolContainer* nonterminalSymbols_;

	FirstSetTable* firstSetContainer_;
	GrammarSymbolSetTable* followSetContainer;

	LR1EdgeTable edges_;
	LR1Itemsets itemsets_;
};
