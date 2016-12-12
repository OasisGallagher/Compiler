#include "lr0.h"
#include "LALR.h"

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

int LRTable::GetNextState(int state, const GrammarSymbol& symbol) {
	int answer = -1;
	gotoTable_.get(state, symbol, answer);
	return answer;
}

std::string LRTable::ToString() const {
	return impl_->ToString();
}