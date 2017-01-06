#pragma once

#include "matrix.h"
#include "grammar_symbol.h"

class GrammarContainer;

struct Forward {
	bool spontaneous;
	GrammarSymbol symbol;

	std::string ToString() const {
		return symbol.ToString();
	}
};

class Forwards {
	typedef std::vector<Forward> container_type;
	IMPLEMENT_REFERENCE_COUNTABLE(Forwards, container_type);
public:
	Forwards();

public:
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;
	typedef container_type::const_reference const_reference;

public:
	bool operator < (const Forwards& other) const;

public:
	const_reference at(int i) const { return ptr_->at(i); }
	int size() const { return ptr_->size(); }
	iterator begin() { return ptr_->begin(); }
	iterator end() { return ptr_->end(); }

	const_iterator begin() const { return ptr_->begin(); }
	const_iterator end() const { return ptr_->end(); }

	bool insert(const GrammarSymbol& symbol, bool spontaneous);
};

struct LR1Item {
	int cpos, dpos;

	Forwards forwards;

	LR1Item(int cp, int dp);
	LR1Item(int cp, int dp, const Forwards& fs);

	bool operator < (const LR1Item& other) const;
	bool operator == (const LR1Item& other) const;

	bool IsCore() const;

	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1Itemset {
	typedef class : public std::set <LR1Item> {
		friend class LR1Itemset;
		std::string name_;
	} container_type;

	IMPLEMENT_REFERENCE_COUNTABLE(LR1Itemset, container_type);

public:
	LR1Itemset();

public:
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;

public:
	iterator begin() { return ptr_->begin(); }
	const_iterator begin() const { return ptr_->begin(); }

	iterator end() { return ptr_->end(); }
	const_iterator end() const { return ptr_->end(); }

	iterator find(const LR1Item& item) { return ptr_->find(item); }
	const_iterator find(const LR1Item& item) const { return ptr_->find(item); }

	void clear() { ptr_->clear(); }
	int size() const { return ptr_->size(); }
	bool empty() const { return ptr_->empty(); }

	bool insert(const LR1Item& item);

public:
	bool operator < (const LR1Itemset& other) const;
	bool operator == (const LR1Itemset& other) const;

public:
	void RemoveNoncoreItems();

	const std::string& GetName() const;
	void SetName(const std::string& name);

	std::string ToString(const GrammarContainer& grammars) const;

private:
#pragma push_macro("new")
#undef new
	// LR1Itemset不可以通过new分配.
	void* operator new(size_t);
#pragma pop_macro("new")
};

class LR1ItemsetContainer : public std::set <LR1Itemset> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class Propagations : public std::map <LR1Itemset, LR1ItemsetContainer> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1EdgeTable : public matrix <LR1Itemset, GrammarSymbol, LR1Itemset> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};
