#include <sstream>
#include <algorithm>

#include "lalr.h"
#include "debug.h"
#include "grammar.h"
#include "table_printer.h"

#define NAME_SEPERATOR "."

LALR::LALR() {
}

LALR::~LALR() {
}

void LALR::Setup(const LRSetupParameter& parameter) {
	grammars_ = parameter.grammars;
	terminalSymbols_ = parameter.terminalSymbols;
	nonterminalSymbols_ = parameter.nonterminalSymbols;

	firstSetContainer_ = parameter.firstSetContainer;
	followSetContainer = parameter.followSetContainer;
}

bool LALR::Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	if (!CreateLR1Itemsets()) {
		return false;
	}

	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
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

bool LALR::CreateLR1Itemsets() {
	LR1Item start(0, 0, GrammarSymbol::zero);
	
	LR1Itemset itemset;
	itemset.SetName("0");

	itemset.insert(start);
	CalculateLR1Itemset(itemset);
	
	itemsets_.insert(itemset);

	for (; CreateLR1ItemsetsOnePass();) {

	}

	Merge();

	return true;
}

void LALR::CalculateLR1Itemset(LR1Itemset& answer) {
	for (; CalculateLR1ItemsetOnePass(answer);) {
	}
}

bool LALR::GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	LR1EdgeTable::iterator pos = edges_.find(src.GetName(), symbol);
	if (pos != edges_.end()) {
		itemsets_.find(pos->second, answer);
		return true;
	}

	CalculateLR1EdgeTarget(answer, src, symbol);
	if (!answer.empty()) {
		edges_.insert(src.GetName(), symbol, answer.GetName());
		return true;
	}

	return false;
}

void LALR::CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	for (LR1Itemset::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		const Condinate* cond = grammars_->GetTargetCondinate(ite->cpos, nullptr);
		SymbolVector::const_iterator pos = std::find(cond->symbols.begin(), cond->symbols.end(), symbol);
		if (pos == cond->symbols.end()) {
			continue;
		}

		if (ite->dpos != (int)std::distance(cond->symbols.begin(), pos)) {
			continue;
		}

		LR1Item item(ite->cpos, ite->dpos + 1, ite->forward);
		answer.insert(item);
	}

	CalculateLR1Itemset(answer);
}

bool LALR::CalculateLR1ItemsetOnePass(LR1Itemset& answer) {
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
				LR1Item newItem(Utility::MakeDword(index, gi), 0, *ite2);
				setChanged = answer.insert(newItem) || setChanged;
			}
		}
	}

	return setChanged;
}

bool LALR::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			itemset.SetName(std::to_string(itemsets_.size()));

			if (GetLR1EdgeTarget(itemset, *ite, ite2->second)) {
				setChanged = itemsets_.insert(itemset) || setChanged;
			}
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			itemset.SetName(std::to_string(itemsets_.size()));

			if (GetLR1EdgeTarget(itemset, *ite, ite2->second)) {
				setChanged = itemsets_.insert(itemset) || setChanged;
			}
		}
	}

	return setChanged;
}

void LALR::Merge() {
	LR1ItemsetVector tmpSets;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		LR1Itemset itemset;
		itemset.SetName(ite->GetName());

		MergeForwardSymbols(itemset, *ite);
		tmpSets.push_back(itemset);
	}

	std::sort(tmpSets.begin(), tmpSets.end());

	MergeItemsets(tmpSets);
}

void LALR::MergeItemsets(const LR1ItemsetVector& itemsets) {
	LR1ItemsetContainer newItemsets;
	LR1ItemsetVector::const_iterator first = itemsets.begin(), pos;

	do {
		pos = Utility::FindGroup(first, itemsets.end(), itemSetCoreComparer);
		LR1Itemset newSet;

		std::string name;
		const char* seperator = "";
		for (LR1ItemsetVector::const_iterator ite = first; ite != pos; ++ite) {
			name += seperator;
			seperator = NAME_SEPERATOR;
			name += ite->GetName();
		}

		newSet.SetName(name);

		MergeNewItemset(newSet, first, pos);

		newItemsets.insert(newSet);

		first = pos;
	} while (first != itemsets.end());

	LR1EdgeTable newEdges;
	RecalculateEdges(newEdges, newItemsets);

	edges_ = newEdges;
	itemsets_ = newItemsets;
}

void LALR::RecalculateEdges(LR1EdgeTable& newEdges, const LR1ItemsetContainer& newItemsets) {
	for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Itemset newTarget;
			RecalculateNewEdgeTarget(newTarget, *ite, ite2->second, newItemsets);
			if (!newTarget.empty()) {
				newEdges.insert(ite->GetName(), ite2->second, newTarget.GetName());
			}
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR1Itemset newTarget;
			RecalculateNewEdgeTarget(newTarget, *ite, ite2->second, newItemsets);
			if (!newTarget.empty()) {
				newEdges.insert(ite->GetName(), ite2->second, newTarget.GetName());
			}
		}
	}
}

void LALR::RecalculateNewEdgeTarget(LR1Itemset& answer, const LR1Itemset& current, const GrammarSymbol& symbol, const LR1ItemsetContainer& newItemsets) {
	std::vector<std::string> states;
	typedef std::vector<std::string>::iterator vsiterator;

	Utility::Split(states, current.GetName(), NAME_SEPERATOR);

	for (vsiterator si = states.begin(); si != states.end(); ++si) {
		LR1Itemset source;
		itemsets_.find(*si, source);
		Assert(!source.empty(), "invalid source state");

		std::string dest;
		if (!edges_.get(source.GetName(), symbol, dest)) {
			continue;
		}

		for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
			if (ite->GetName().find(dest) != std::string::npos) {
				answer = *ite;
				break;
			}
		}
	}
}

void LALR::MergeNewItemset(LR1Itemset &newSet, LR1ItemsetVector::const_iterator first, LR1ItemsetVector::const_iterator last) {
	for (int i = 0; i < first->size(); ++i) {
		LR1Itemset::iterator current = first->begin();
		std::advance(current, i);

		LR1Item newItem(current->cpos, current->dpos, nullptr, new SymbolVector);

		for (LR1ItemsetVector::const_iterator ite = first; ite != last; ++ite) {
			LR1Itemset::const_iterator ite2 = ite->begin();
			std::advance(ite2, i);

			for (Forwards::const_iterator svi = ite2->forwards.begin(); svi != ite2->forwards.end(); ++svi) {
				newItem.forwards.insert(*svi);
			}
		}

		newSet.insert(newItem);
	}
}

void LALR::MergeForwardSymbols(LR1Itemset& answer, const LR1Itemset& itemset) {
	Assert(!itemset.empty(), "empty itemset");

	LR1Itemset::const_iterator first = itemset.begin(), pos;
	do {
		pos = Utility::FindGroup(first, itemset.end(), itemCoreComparer);
		LR1Item newItem(first->cpos, first->dpos, nullptr, new SymbolVector);

		for (; first != pos; ++first) {
			const LR1Item& item = *first;
			newItem.forwards.insert(item.forward);
		}

		answer.insert(newItem);
		first = pos;
	} while (first != itemset.end());
}

std::string LALR::ToString() const {
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

	for (LR1ItemsetContainer::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << ite->GetName();
		std::string dest;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			edges_.get(ite->GetName(), ite2->second, dest);
			tp << dest;
		}

		for (GrammarSymbolContainer::const_iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			edges_.get(ite->GetName(), ite2->second, dest);
			tp << dest;
		}
	}

	tp.AddFooter();

	oss << tp.ToString();

	return oss.str();
}
