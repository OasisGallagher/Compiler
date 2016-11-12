#pragma once

#include "lr_impl.h"

class LRTable {
public:
	LRTable();
	~LRTable();

public:
	bool Create(GrammarContainer* grammars, GrammarSymbolContainer* terminalSymbols, GrammarSymbolContainer* nonterminalSymbols);

	LRAction GetAction(int state, const GrammarSymbol& symbol);
	int GetNextState(int state, const GrammarSymbol& symbol);

	std::string ToString() const;

private:
	LRImpl* impl_;

	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
