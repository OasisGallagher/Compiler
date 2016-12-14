#include <sstream>

#include "debug.h"
#include "lr_impl.h"
#include "grammar_symbol.h"

static const char* actionTexts[] = { "err", "s", "r", "acc" };

bool LRAction::operator == (const LRAction& other) const {
	return actionType == other.actionType && actionParameter == other.actionParameter;
}

bool LRAction::operator != (const LRAction& other) const {
	return actionType != other.actionType || actionParameter != other.actionParameter;
}

std::string LRAction::ToString() const {
	std::ostringstream oss;
	oss << "(";
	oss << actionTexts[actionType];

	if (actionType == LRActionReduce || actionType == LRActionShift) {
		oss << actionParameter;
	}

	oss << ")";

	return oss.str();
}

std::string LRGotoTable::ToString() const {
	const char* seperator = "";
	std::ostringstream oss;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(";
		oss << ite->first.first;
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << ")";
		oss << " => ";
		oss << ite->second;
	}

	return oss.str();
}

std::string LRActionTable::ToString() const {
	const char* seperator = "";
	std::ostringstream oss;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(";
		oss << ite->first.first;
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << ")";
		oss << " => ";
		oss << ite->second.ToString();
	}

	return oss.str();
}

bool LRImpl::InsertActionTable(LRActionTable &actionTable, int src, const GrammarSymbol& symbol, const LRAction& action) {
	LRActionTable::insert_status status = actionTable.insert(src, symbol, action);
	if (!status.second && status.first->second != action) {
		std::ostringstream oss;
		oss << "CONFLICT: (";
		oss << src << ", " << symbol.ToString();
		oss << ")";
		oss << " => ";
		oss << "(";
		oss << status.first->second.ToString() << ", " << action.ToString();
		oss << ")";
		Debug::LogWarning(oss.str());
	}

	return status.second;
}
