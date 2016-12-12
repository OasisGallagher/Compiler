#include <sstream>

#include "lr1.h"
#include "debug.h"
#include "grammar.h"

bool Forwards::insert(const GrammarSymbol& symbol) {
	if (std::find(begin(), end(), symbol) == end()) {
		ptr_->push_back(symbol);
		return true;
	}

	return false;
}

LR1Item::LR1Item(int condinatepos, int dotpos, const GrammarSymbol& forwardSymbol, SymbolVector* forwardSymbols)
	: cpos(condinatepos), dpos(dotpos), forward(forwardSymbol), forwards(forwardSymbols) {
}

bool LR1Item::operator < (const LR1Item& other) const {
	if (cpos == other.cpos) {
		if (dpos == other.dpos) {
			return forward < other.forward;
		}

		return dpos < other.dpos;
	}

	return cpos < other.cpos;
}

bool LR1Item::operator ==(const LR1Item& other) const {
	return cpos == other.cpos && dpos == other.dpos && forward == other.forward;
}

std::string LR1Item::ToString(const GrammarContainer& grammars) const {
	Grammar* g = nullptr;
	const Condinate* cond = grammars.GetTargetCondinate(cpos, &g);

	std::ostringstream oss;
	oss << g->GetLhs().ToString() << " : ";

	const char* seperator = "";
	for (size_t i = 0; i < cond->symbols.size(); ++i) {
		if (i == dpos) {
			oss << seperator;
			seperator = " ";
			oss << "¡¤";
		}

		oss << seperator;
		oss << cond->symbols[i].ToString();
		seperator = " ";
	}

	if (dpos == (int)cond->symbols.size()) {
		oss << seperator << "¡¤";
	}

	oss << ", ";
	if (forward) {
		oss << forward.ToString();
	}
	else {
		oss << "( " << Utility::Concat(forwards.begin(), forwards.end(), "/") << " )";
	}

	return oss.str();
}

LR1Itemset::LR1Itemset() {
	ptr_ = new container_type;
}

bool LR1Itemset::operator < (const LR1Itemset& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		if (!(*first1 == *first2)) {
			return *first1 < *first2;
		}
	}

	if (first1 == end() && first2 == other.end()) {
		return false;
	}

	return first1 == end();
}

bool LR1Itemset::operator == (const LR1Itemset& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		if (!(*first1 == *first2)) {
			return false;
		}
	}

	return first1 == end() && first2 == other.end();
}

const std::string& LR1Itemset::GetName() const {
	Assert(!ptr_->name_.empty(), "invalid name");
	return ptr_->name_;
}

void LR1Itemset::SetName(const std::string& name) {
	ptr_->name_ = name;
}

std::string LR1Itemset::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	oss << "(" << GetName() << ")\t";
	oss << "{ ";

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = ", ";
		oss << "{ " << ite->ToString(grammars) << " }";
	}

	oss << " }";

	return oss.str();
}

bool LR1ItemsetContainer::find(const std::string& name, LR1Itemset& answer) {
	// TODO:
	for (iterator ite = begin(); ite != end(); ++ite) {
		if (ite->GetName() == name) {
			answer = *ite;
			return true;
		}
	}

	return false;
}

std::string LR1ItemsetContainer::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	const char* seperator = "";
	int index = 0;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << ite->ToString(grammars);
	}

	return oss.str();
}

std::string LR1EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		oss << "( ";
		oss << ite->first.first;
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << " )";

		oss << " => ";
		oss << ite->second;
	}

	return oss.str();
}
