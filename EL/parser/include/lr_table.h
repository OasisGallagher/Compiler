#pragma once
#include "matrix.h"
#include "lr_impl.h"

class LRGotoTable : public matrix <int, GrammarSymbol, int> {
public:
	std::string ToString() const;
};

class LRActionTable : public matrix <int, GrammarSymbol, LRAction> {
public:
	std::string ToString(const GrammarContainer& grammars) const;
};

class LRTable {
public:
	LRTable();
	~LRTable();

public:
	friend class Serializer;
	friend class LRParser;

public:
	int GetGoto(int current, const GrammarSymbol& symbol);
	LRAction GetAction(int current, const GrammarSymbol& symbol);
	
	std::string ToString(const GrammarContainer& grammars) const;

private:
	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
