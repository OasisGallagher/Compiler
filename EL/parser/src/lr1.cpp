#include <sstream>
#include <algorithm>

#include "lr1.h"
#include "debug.h"
#include "table_printer.h"

struct ItemComp {
	bool operator ()(const LR1Item& lhs, const LR1Item& rhs) const {
		return lhs.cpos == rhs.cpos && lhs.dpos == rhs.dpos;
	}
};

static ItemComp itemComp;

struct ItemSetComp {
	bool operator ()(const LR1Itemset& lhs, const LR1Itemset& rhs) const {
		return !(lhs < rhs) && !(rhs < lhs);
	}
};

static ItemSetComp itemSetComp;

LR1::LR1() {
}

LR1::~LR1() {
}

void LR1::Setup(const LRSetupParameter& parameter) {
	grammars_ = parameter.grammars;
	terminalSymbols_ = parameter.terminalSymbols;
	nonterminalSymbols_ = parameter.nonterminalSymbols;

	firstSetContainer_ = parameter.firstSetContainer;
	followSetContainer = parameter.followSetContainer;
}

bool LR1::Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	if (!CreateLR1Itemsets()) {
		return false;
	}

	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LR1::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		const LR1Itemset& itemset = *ite;
		for (LR1Itemset::const_iterator ite2 = itemset.begin(); ite2 != itemset.end(); ++ite2) {
			const LR1Item& item = *ite2;
			const Condinate* cond = grammars_->GetTargetCondinate(item.cpos, nullptr);

			if (item.dpos >= (int)cond->symbols.size()) {
				continue;
			}

			const GrammarSymbol& symbol = cond->symbols[item.dpos];
		}
	}
	return true;
}

std::string LR1::ToString() const {
	std::ostringstream oss;

	oss << Utility::Heading(" LR1 Edges ") << "\n";
	oss << edges_.ToString(*grammars_);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 Itemsets ") << "\n";
	oss << itemsets_.ToString(*grammars_);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 DFA ") << "\n";
	TablePrinter tp;

	int maxlength = 0;
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols_->begin(); ite != terminalSymbols_->end(); ++ite) {
		if ((int)ite->first.length() > maxlength) {
			maxlength = ite->first.length();
		}
	}

	tp.AddColumn("", maxlength + 2);
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols_->begin(); ite != terminalSymbols_->end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	for (GrammarSymbolContainer::const_iterator ite = nonterminalSymbols_->begin(); ite != nonterminalSymbols_->end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	tp.AddHeader();

	int si = 0;
	for (LR1ItemsetContainer::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << si++;
		LR1Itemset dest;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, dest)) {
				tp << (int)std::distance(itemsets_.begin(), itemsets_.find(dest));
			}
			else {
				tp << "";
			}
		}

		for (GrammarSymbolContainer::const_iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, dest)) {
				tp << (int)std::distance(itemsets_.begin(), itemsets_.find(dest));
			}
			else {
				tp << "";
			}
		}
	}

	tp.AddFooter();

	oss << tp.ToString();

	return oss.str();
}

bool LR1::CreateLR1Itemsets() {
	LR1Item start = { 0, 0, GrammarSymbol::zero };
	
	LR1Itemset itemset;
	itemset.insert(start);
	CalculateLR1Itemset(itemset);

	itemsets_.insert(itemset);

	for (; CreateLR1ItemsetsOnePass();) {

	}

	itemsets_.Merge();

	return true;
}

void LR1::CalculateLR1Itemset(LR1Itemset& answer) {
	for (; CalculateLR1ItemsetOnePass(answer);) {
	}
}

bool LR1::GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	LR1EdgeTable::iterator pos = edges_.find(src, symbol);
	if (pos != edges_.end()) {
		answer = pos->second;
		return true;
	}

	CalculateLR1EdgeTarget(answer, src, symbol);
	if (!answer.empty()) {
		edges_.insert(src, symbol, answer);
		return true;
	}

	return false;
}

void LR1::CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	for (LR1Itemset::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		const Condinate* cond = grammars_->GetTargetCondinate(ite->cpos, nullptr);
		SymbolVector::const_iterator pos = std::find(cond->symbols.begin(), cond->symbols.end(), symbol);
		if (pos == cond->symbols.end()) {
			continue;
		}

		if (ite->dpos != (int)std::distance(cond->symbols.begin(), pos)) {
			continue;
		}

		LR1Item item = { ite->cpos, ite->dpos + 1, ite->forward };
		answer.insert(item);
	}

	CalculateLR1Itemset(answer);
}

bool LR1::CalculateLR1ItemsetOnePass(LR1Itemset& answer) {
	bool setChanged = false;
	for (LR1Itemset::iterator ite = answer.begin(); ite != answer.end(); ++ite) {
		const LR1Item& current = *ite;
		const Condinate* cond = grammars_->GetTargetCondinate(current.cpos, nullptr);
		if (current.dpos >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& b = cond->symbols[current.dpos];

		if (b.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		int gi = 1;
		const CondinateContainer& conds = grammars_->FindGrammar(b, &gi)->GetCondinates();

		SymbolVector beta(cond->symbols.begin() + current.dpos + 1, cond->symbols.end());
		beta.push_back(current.forward);

		GrammarSymbolSet firstSet;
		firstSetContainer_->GetFirstSet(firstSet, beta.begin(), beta.end());

		for (GrammarSymbolSet::iterator ite2 = firstSet.begin(); ite2 != firstSet.end(); ++ite2) {
			if (ite2->SymbolType() == GrammarSymbolNonterminal) {
				continue;
			}

			int index = 0;
			for (CondinateContainer::const_iterator ite3 = conds.begin(); ite3 != conds.end(); ++ite3, ++index) {
				LR1Item newItem = { Utility::MakeDword(index, gi), 0, *ite2 };
				setChanged = answer.insert(newItem) || setChanged;
			}
		}
	}

	return setChanged;
}

bool LR1::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			if (GetLR1EdgeTarget(itemset, *ite, ite2->second)) {
				setChanged = itemsets_.insert(itemset).second || setChanged;
			}
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			if (GetLR1EdgeTarget(itemset, *ite, ite2->second)) {
				setChanged = itemsets_.insert(itemset).second || setChanged;
			}
		}
	}

	return setChanged;
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
		oss << "( " << Utility::Concat(container->begin(), container->end(), "/") << " )";
	}

	return oss.str();
}

LR1Itemset::LR1Itemset() {
	ptr_ = new LR1Closure;
}

LR1Itemset::iterator LR1Itemset::begin() {
	return ptr_->begin();
}

LR1Itemset::const_iterator LR1Itemset::begin() const {
	return ptr_->begin();
}

LR1Itemset::iterator LR1Itemset::end() {
	return ptr_->end();
}

LR1Itemset::const_iterator LR1Itemset::end() const {
	return ptr_->end();
}

void LR1Itemset::clear() {
	ptr_->clear();
}

bool LR1Itemset::empty() const {
	return ptr_->empty();
}

int LR1Itemset::size() const {
	return (int)ptr_->size();
}

bool LR1Itemset::insert(const LR1Item& item) {
	return ptr_->insert(item).second;
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

std::string LR1Itemset::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	oss << "{ ";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = ", ";
		oss << "{ " << ite->ToString(grammars) << " }";
	}
	oss << " }";

	return oss.str();
}

void LR1ItemsetContainer::Merge() {
	std::vector<LR1Itemset> container;
	for (iterator ite = begin(); ite != end(); ++ite) {
		LR1Itemset itemset;
		MergeItemset(itemset, *ite);
		container.push_back(itemset);
	}

	clear();
	std::sort(container.begin(), container.end());

	std::vector<LR1Itemset>::iterator first = container.begin(), pos;
	LR1Itemset::iterator current;

	do {
		pos = Utility::FindGroup(first, container.end(), itemSetComp);

		LR1Itemset newSet;
		for (int i = 0; i < first->size(); ++i) {
			current = first->begin();
			std::advance(current, i);

			LR1Item newItem = *current;
			newItem.container = new SymbolVector;

			for (std::vector<LR1Itemset>::iterator ite = first; ite != pos; ++ite) {
				LR1Itemset::const_iterator ite2 = ite->begin();
				std::advance(ite2, i);
				for (SymbolVector::iterator svi = ite2->container->begin(); svi != ite2->container->end(); ++svi) {
					if (std::find(newItem.container->begin(), newItem.container->end(), *svi) == newItem.container->end()) {
						newItem.container->push_back(*svi);
					}
				}
			}

			newSet.insert(newItem);
		}

		insert(newSet);

		first = pos;
	} while (first != container.end());
}

void LR1ItemsetContainer::MergeItemset(LR1Itemset& answer, const LR1Itemset& itemset) {
	Assert(!itemset.empty(), "empty itemset");

	LR1Itemset::const_iterator first = itemset.begin(), pos;
	do {
		pos = Utility::FindGroup(first, itemset.end(), itemComp);
		LR1Item newItem = *first;
		newItem.forward = nullptr;

		newItem.container = new SymbolVector;
		MergeForwardSymbols(newItem.container, first, pos);

		answer.insert(newItem);
		first = pos;
	} while (first != itemset.end());
}

void LR1ItemsetContainer::MergeForwardSymbols(SymbolVector* dest, LR1Itemset::const_iterator first, LR1Itemset::const_iterator last) {
	int dist = (int)std::distance(first, last);
	for (; first != last; ++first) {
		const LR1Item& item = *first;
		if (std::find(dest->begin(), dest->end(), item.forward) == dest->end()) {
			dest->push_back(item.forward);
		}
	}
}

std::string LR1ItemsetContainer::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	const char* seperator = "";
	int index = 0;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(" << index++ << ")\t" << ite->ToString(grammars);
	}

	return oss.str();
}

std::string LR1EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		const LR1Itemset& src = ite->first.first;
		const GrammarSymbol& symbol = ite->first.second;
		const LR1Itemset& dest = ite->second;

		oss << "( ";
		oss << src.ToString(grammars);
		oss << ", ";
		oss << symbol.ToString();
		oss << " )";

		oss << " => ";
		oss << "{ " << dest.ToString(grammars) << " }";
	}

	return oss.str();
}
