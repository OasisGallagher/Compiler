#pragma once

#include "lr_impl.h"

class LRTable {
public:
	LRTable();
	~LRTable();

public:
	bool Create(const LRSetupParameter& parameter);

	LRAction GetAction(int state, const GrammarSymbol& symbol);
	int GetNextGotoState(int state, const GrammarSymbol& symbol);

	std::string ToString() const;

private:
	LRImpl* impl_;

	LRGotoTable gotoTable_;
	LRActionTable actionTable_;
};
