#pragma once

#include "matrix.h"
#include "grammar_symbol.h"

class GrammarContainer;

class Forwards {
	IMPLEMENT_REFERENCE_COUNTABLE(Forwards, SymbolVector);
public:
	Forwards();

public:
	typedef SymbolVector::iterator iterator;
	typedef SymbolVector::const_iterator const_iterator;
	typedef SymbolVector::const_reference const_reference;

public:
	bool operator < (const Forwards& other) const;

public:
	const_reference at(int i) const { return ptr_->at(i); }
	int size() const { return ptr_->size(); }
	iterator begin() { return ptr_->begin(); }
	iterator end() { return ptr_->end(); }

	const_iterator begin() const { return ptr_->begin(); }
	const_iterator end() const { return ptr_->end(); }

	bool insert(const GrammarSymbol& symbol);
};

struct LR1Item {
	int cpos, dpos;

	Forwards forwards;

	LR1Item(int cp, int dp);
	LR1Item(int cp, int dp, const Forwards& fs);

	bool operator < (const LR1Item& other) const;
	bool operator == (const LR1Item& other) const;

	std::string ToString(const GrammarContainer& grammars) const;
};

class LR1ItemsetName {
	IMPLEMENT_REFERENCE_COUNTABLE(LR1ItemsetName, std::string);

public:
	LR1ItemsetName();
	LR1ItemsetName(const char* name);
	LR1ItemsetName(const std::string& name);

public:
	bool operator < (const LR1ItemsetName& other) const { return *ptr_ < *other.ptr_; }
	bool operator == (const LR1ItemsetName& other) const { return *ptr_ == *other.ptr_; }

public:
	bool empty() const { return ptr_->empty(); }
	void assign(const std::string& other) { ptr_->assign(other); }
	const std::string& ToString() const { return *ptr_; }
};

class LR1Itemset {
	typedef class : public std::set <LR1Item> {
		friend class LR1Itemset;
		LR1ItemsetName name_;
	} container_type;

	IMPLEMENT_REFERENCE_COUNTABLE(LR1Itemset, container_type);

public:
	LR1Itemset();

public:
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;

public:
	iterator begin() { return ptr_->begin(); }
	iterator end() { return ptr_->end(); }
	const_iterator begin() const { return ptr_->begin(); }
	const_iterator end() const { return ptr_->end(); }

	void clear() { ptr_->clear(); }
	int size() const { return ptr_->size(); }
	bool empty() const { return ptr_->empty(); }

	bool insert(const LR1Item& item);

public:
	bool operator < (const LR1Itemset& other) const;
	bool operator == (const LR1Itemset& other) const;

public:
	const LR1ItemsetName& GetName() const;

	void SetName(const char* name); 
	void SetName(const std::string& name);
	void SetName(const LR1ItemsetName& name);
	std::string ToString(const GrammarContainer& grammars) const;

private:
#pragma push_macro("new")
#undef new
	// LR1Itemset不可以通过new分配.
	void* operator new(size_t);
#pragma pop_macro("new")
};

typedef std::vector<LR1Itemset> LR1ItemsetVector;
struct ItemSetComparer {
	bool operator ()(const LR1Itemset& lhs, const LR1Itemset& rhs) const;
	bool CompareItemSet(const LR1Item& lhs, const LR1Item& rhs) const;
};

class LR1ItemsetContainer {
	typedef ItemSetComparer comparer_type;
	typedef std::set<LR1Itemset, comparer_type> container_type;

public:
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;

public:
	iterator begin() { return container_.begin(); }
	iterator end() { return container_.end(); }

	const_iterator begin() const { return container_.begin(); }
	const_iterator end() const { return container_.end(); }

	void clear() { container_.clear(); }

	int size() const { return (int)container_.size(); }

	bool find(const std::string& name, LR1Itemset& answer);

	typedef std::pair<iterator, bool> insert_status;
	insert_status insert(const LR1Itemset& itemset) { return container_.insert(itemset); }

public:
	std::string ToString(const GrammarContainer& grammars) const;

private:
	container_type container_;
};

class LR1EdgeTable : public matrix <LR1ItemsetName, GrammarSymbol, LR1ItemsetName> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};
