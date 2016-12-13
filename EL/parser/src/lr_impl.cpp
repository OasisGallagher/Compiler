#include <sstream>

#include "lr_impl.h"
#include "grammar_symbol.h"

static const char* actionTexts[] = { "err", "s", "r", "acc" };

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
