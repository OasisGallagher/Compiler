#include <sstream>

#include "lalr.h"
#include "lr_table.h"
#include "grammar_symbol.h"

LRTable::LRTable() {
	impl_ = new LALR();
}

LRTable::~LRTable() {
	delete impl_;
}

bool LRTable::Create(const LRSetupParameter& parameter) {
	impl_->Setup(parameter);
	return impl_->Parse(gotoTable_, actionTable_);
}

LRAction LRTable::GetAction(int state, const GrammarSymbol& symbol) {
	LRAction action = { LRActionError };
	actionTable_.get(state, symbol, action);
	return action;
}

int LRTable::GetNextGotoState(int state, const GrammarSymbol& symbol) {
	int answer = -1;
	gotoTable_.get(state, symbol, answer);
	return answer;
}

std::string LRTable::ToString() const {
	std::ostringstream oss;
	oss << impl_->ToString();

	oss << Utility::Heading(" Action Table ") << "\n";
	oss << actionTable_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" Goto Table ") << "\n";
	oss << gotoTable_.ToString();

	return oss.str();
}