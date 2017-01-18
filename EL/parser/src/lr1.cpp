#include <sstream>
#include <algorithm>

#include "lr1.h"
#include "debug.h"
#include "define.h"
#include "grammar.h"

Forwards::Forwards() {
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

void Forwards::erase(const GrammarSymbol& symbol) {
	iterator last = begin();
	for (iterator first = begin(); first != end(); ++first) {
		if (first->symbol != symbol) {
			*last++ = *first;
		}
	}

	cont_.erase(last, end());
}

bool Forwards::insert(const GrammarSymbol& symbol, bool spontaneous) {
	for (iterator ite = begin(); ite != end(); ++ite) {
		if (ite->symbol == symbol) {
			return false;
		}
	}

	Forward f = { spontaneous, symbol };
	cont_.push_back(f);
	return true;
}

LR1Item::LR1Item() {
	ptr_ = new Impl;
	ptr_->cpos = 0;
	ptr_->dpos = 0;
}

LR1Item::LR1Item(int cpos, int dpos) {
	ptr_ = new Impl;
	ptr_->cpos = cpos;
	ptr_->dpos = dpos;
}

LR1Item::LR1Item(int cpos, int dpos, const Forwards& forwards) {
	ptr_ = new Impl;
	ptr_->cpos = cpos;
	ptr_->dpos = dpos;
	ptr_->forwards = forwards;
}

bool LR1Item::operator < (const LR1Item& other) const {
	if (ptr_->cpos == other.ptr_->cpos) {
		return ptr_->dpos < other.ptr_->dpos;
	}

	return ptr_->cpos < other.ptr_->cpos;
}

bool LR1Item::operator ==(const LR1Item& other) const {
	return ptr_->cpos == other.ptr_->cpos && ptr_->dpos == other.ptr_->dpos;
}

bool LR1Item::IsCore() const {
	return ptr_->dpos != 0 || ptr_->cpos == 0;
}

std::string LR1Item::ToRawString() const {
	return Utility::Format("(%d, %d, %d)", Utility::Highword(ptr_->cpos), Utility::Loword(ptr_->cpos), ptr_->dpos);
}

std::string LR1Item::ToString(const GrammarContainer& grammars) const {
	Grammar* g = nullptr;
	const Condinate* cond = grammars.GetTargetCondinate(ptr_->cpos, &g);

	std::ostringstream oss;
	oss << g->GetLhs().ToString() << " : ";

	const char* seperator = "";
	for (size_t i = 0; i < cond->symbols.size(); ++i) {
		if (i == ptr_->dpos) {
			oss << seperator;
			seperator = " ";
			oss << "¡¤";
		}

		oss << seperator;
		oss << cond->symbols[i].ToString();
		seperator = " ";
	}

	if (ptr_->dpos == (int)cond->symbols.size()) {
		oss << seperator << "¡¤";
	}

	oss << ", ";
	oss << "( " << Utility::Concat(ptr_->forwards.begin(), ptr_->forwards.end(), "/") << " )";

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
	const Forwards& forwards = item.GetForwards();
	for (Forwards::const_iterator ite = forwards.begin(); ite != forwards.end(); ++ite) {
		result = old.GetForwards().insert(ite->symbol, ite->spontaneous) || result;
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
		if (!ite->IsCore()) {
			continue;
		}

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

std::string Propagations::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << ite->first.ToString(grammars) << " >> ( ";
		const char* seperator2 = "";
		for (LR1Itemset::const_iterator ite2 = ite->second.begin();
			ite2 != ite->second.end(); ++ite2) {
			oss << seperator2;
			seperator2 = ", ";
			oss << ite2->ToString(grammars);
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
		oss << ite->first.first.GetName();
		oss << ", ";
		oss << ite->first.second.ToString();
		oss << " )";

		oss << " => ";
		oss << ite->second.GetName();
	}

	return oss.str();
}
