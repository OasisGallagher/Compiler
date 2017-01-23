#include <sstream>

#include "lalr.h"
#include "lr_table.h"
#include "grammar_symbol.h"

LRTable::LRTable() {
}

LRTable::~LRTable() {
}

LRAction LRTable::GetAction(int current, const GrammarSymbol& symbol) {
	LRAction action = { LRActionError };
	actionTable_.get(current, symbol, action);
	return action;
}

int LRTable::GetGoto(int current, const GrammarSymbol& symbol) {
	int answer = -1;
	gotoTable_.get(current, symbol, answer);
	return answer;
}

std::string LRTable::ToString() const {
	std::ostringstream oss;
	
	oss << Utility::Heading(" Action Table ") << "\n";
	oss << actionTable_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" Goto Table ") << "\n";
	oss << gotoTable_.ToString();

	return oss.str();
}
