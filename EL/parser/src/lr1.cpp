#include <sstream>

#include "lr1.h"
#include "debug.h"
#include "define.h"
#include "grammar.h"

Forwards::Forwards() {
	ptr_ = new container_type;
}

bool Forwards::operator <(const Forwards& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		if (first1->symbol != first2->symbol) {
			return first1->symbol < first2->symbol;
		}
	}

	if (first1 == end() && first2 == other.end()) {
		return false;
	}

	return first1 == end();
}

bool Forwards::insert(const GrammarSymbol& symbol, bool spontaneous) {
	for (iterator ite = begin(); ite != end(); ++ite) {
		if (ite->symbol == symbol) {
			return false;
		}
	}

	Forward f = { spontaneous, symbol };
	ptr_->push_back(f);
	return true;
}

LR1Item::LR1Item(int cp, int dp)
	: cpos(cp), dpos(dp) {

}

LR1Item::LR1Item(int cp, int dp, const Forwards& fs)
	: cpos(cp), dpos(dp), forwards(fs) {
}

bool LR1Item::operator < (const LR1Item& other) const {
	if (cpos == other.cpos) {
		return dpos < other.dpos;
	}

	return cpos < other.cpos;
}

bool LR1Item::operator ==(const LR1Item& other) const {
	return cpos == other.cpos && dpos == other.dpos;
}

bool LR1Item::IsCore() const {
	return dpos != 0 || cpos == 0;
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
	oss << "( " << Utility::Concat(forwards.begin(), forwards.end(), "/") << " )";

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

bool LR1Itemset::insert(const LR1Item& item) {
	std::pair<iterator, bool> state = ptr_->insert(item);
	if (state.second) {
		return true;
	}

	bool result = false;

	LR1Item& old = (LR1Item&)*state.first;
	for (Forwards::const_iterator ite = item.forwards.begin(); ite != item.forwards.end(); ++ite) {
		result = old.forwards.insert(ite->symbol, ite->self) || result;
	}

	return result;
}

const std::string& LR1Itemset::GetName() const {
	return ptr_->name_;
}

void LR1Itemset::SetName(const std::string& name) {
	ptr_->name_ = name;
}

void LR1Itemset::RemoveNoncoreItems() {
	for (iterator ite = ptr_->begin(); ite != ptr_->end();) {
		if (!ite->IsCore()) {
			ptr_->erase(ite++);
		}
		else {
			ite++;
		}
	}
}

std::string LR1Itemset::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	oss << "(" << GetName() << ") ";
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

std::string SpreadDictionary::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << ite->first.GetName() << " >> ( ";
		const char* seperator2 = "";
		for (LR1ItemsetContainer::const_iterator ite2 = ite->second.begin();
			ite2 != ite->second.end(); ++ite2) {
			oss << seperator2;
			seperator2 = ", ";
			oss << ite2->GetName();
		}

		oss << " )";
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
		oss << ite->first.first.GetName();// ToString(grammars);
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << " )";

		oss << " => ";
		oss << ite->second.GetName();// ToString(grammars);
	}

	return oss.str();
}
