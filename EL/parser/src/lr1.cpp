#include <sstream>

#include "lr1.h"
#include "debug.h"
#include "grammar.h"

Forwards::Forwards() {
	ptr_ = new SymbolVector;
}

bool Forwards::operator <(const Forwards& other) const {
	const_iterator first1 = begin(), first2 = other.begin();
	for (; first1 != end() && first2 != other.end(); ++first1, ++first2) {
		if (*first1 != *first2) {
			return *first1 < *first2;
		}
	}

	if (first1 == end() && first2 == other.end()) {
		return false;
	}

	return first1 == end();
}

bool Forwards::insert(const GrammarSymbol& symbol) {
	if (std::find(begin(), end(), symbol) == end()) {
		ptr_->push_back(symbol);
		return true;
	}

	return false;
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

LR1ItemsetName::LR1ItemsetName() {
	ptr_ = new std::string;
}

LR1ItemsetName::LR1ItemsetName(const char* name) {
	ptr_ = new std::string(name);
}

LR1ItemsetName::LR1ItemsetName(const std::string& name) {
	ptr_ = new std::string(name);
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
	for (SymbolVector::const_iterator ite = item.forwards.begin(); ite != item.forwards.end(); ++ite) {
		result = old.forwards.insert(*ite) || result;
	}

	return result;
}

const LR1ItemsetName& LR1Itemset::GetName() const {
	return ptr_->name_;
}

void LR1Itemset::SetName(const char* name_) {
	ptr_->name_.assign(name_);
}

void LR1Itemset::SetName(const std::string& name) {
	ptr_->name_.assign(name);
}

void LR1Itemset::SetName(const LR1ItemsetName& name) {
	ptr_->name_ = name;
}

std::string LR1Itemset::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	oss << "(" << GetName().ToString() << ")\t";
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

LR1Itemset& LR1ItemsetContainer::operator[](const std::string& name) {
	dictionary::iterator pos = dict_.find(name);
	Assert(pos != dict_.end(), "can not find item named " + name);
	return pos->second;
}

LR1Itemset& LR1ItemsetContainer::operator[](const LR1ItemsetName& name) {
	return operator[](name.ToString());
}

bool LR1ItemsetContainer::insert(LR1Itemset& itemset) {
	std::pair<iterator, bool> status = container_.insert(itemset);
	if (itemset.GetName().empty()) {
		if (status.second) {
			itemset.SetName(std::to_string(size() - 1));
		}
		else {
			itemset.SetName(status.first->GetName());
		}
	}

	dict_.insert(std::make_pair(itemset.GetName().ToString(), itemset));
	return status.second;
}

LR1ItemsetContainer& LR1ItemsetContainer::operator = (const LR1ItemsetContainer& other) {
	container_ = other.container_;
	
	dict_.clear();
	//for (LR1ItemsetContainer::const_iterator ite = other.begin(); ite != other.end(); ++ite) {
	//	dict_.insert(std::make_pair(ite->GetName(), *ite));
	//}

	return *this;
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

bool ItemSetComparer::operator()(const LR1Itemset& lhs, const LR1Itemset& rhs) const {
	LR1Itemset::const_iterator first1 = lhs.begin(), first2 = rhs.begin();
	for (; first1 != lhs.end() && first2 != rhs.end(); ++first1, ++first2) {
		if (CompareItemSet(*first1, *first2) || CompareItemSet(*first2, *first1)) {
			return CompareItemSet(*first1, *first2);
		}
	}

	if (first1 == lhs.end() && first2 == rhs.end()) {
		return false;
	}

	return first1 == lhs.end();
}

bool ItemSetComparer::CompareItemSet(const LR1Item& lhs, const LR1Item& rhs) const {
	if (lhs.cpos == rhs.cpos) {
		if (lhs.dpos == rhs.dpos) {
			return lhs.forwards < rhs.forwards;
		}

		return lhs.dpos < rhs.dpos;
	}

	return lhs.cpos < rhs.cpos;
}

std::string LR1EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		oss << "( ";
		oss << ite->first.first.ToString();
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << " )";

		oss << " => ";
		oss << ite->second.ToString();
	}

	return oss.str();
}
