#include <sstream>
#include <algorithm>

#include "lalr.h"
#include "debug.h"
#include "grammar.h"
#include "table_printer.h"

#define NAME_SEPERATOR '.'

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

	Debug::Log("Create parsing table.");

	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	return CreateActionTable(actionTable) && CreateGotoTable(gotoTable);
}

bool LALR::CreateLR1Itemsets() {
	LR1Item start(0, 0);
	start.forwards.insert(GrammarSymbol::zero);
	
	LR1Itemset itemset;
	itemset.SetName("0");

	itemset.insert(start);
	CalculateLR1Itemset(itemset);
	
	itemsets_.insert(itemset);

	Debug::Log("Create itemsets.");
	for (; CreateLR1ItemsetsOnePass();) {
	}

	Debug::Log("Merge itemsets.");
	MergeItemsets();

	return true;
}

bool LALR::CreateActionTable(LRActionTable &actionTable) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		const LR1Itemset& itemset = *ite;
		for (LR1Itemset::const_iterator ite2 = itemset.begin(); ite2 != itemset.end(); ++ite2) {
			Grammar* g = nullptr;
			const LR1Item& item = *ite2;
			const Condinate* cond = grammars_->GetTargetCondinate(item.cpos, &g);
			int i = 0;
			Utility::ParseInteger(itemset.GetName().ToString(), &i);

			if (g->GetLhs() == GrammarSymbol::program && item.dpos == 1) {
				Assert(item.forwards.size() == 1 && *item.forwards.begin() == GrammarSymbol::zero, "invalid start state");
				LRAction action = { LRActionAccept };
				InsertActionTable(actionTable, i, GrammarSymbol::zero, action);
				continue;
			}

			if (item.dpos >= (int)cond->symbols.size()) {
				if (g->GetLhs() != GrammarSymbol::program) {
					LRAction action = { LRActionReduce, item.cpos };
					for (Forwards::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
						InsertActionTable(actionTable, i, *fi, action);
					}
				}

				continue;
			}

			const GrammarSymbol& symbol = cond->symbols[item.dpos];
			if (symbol.SymbolType() != GrammarSymbolTerminal) {
				continue;
			}

			LR1ItemsetName target;
			if (edges_.get(itemset.GetName(), symbol, target)) {
				int j = 0;
				Utility::ParseInteger(target.ToString(), &j);
				LRAction action = { LRActionShift, j };
				InsertActionTable(actionTable, i, symbol, action);
			}
		}
	}

	return true;
}

bool LALR::CreateGotoTable(LRGotoTable &gotoTable) {
	for (LR1EdgeTable::const_iterator ite = edges_.begin(); ite != edges_.end(); ++ite) {
		const GrammarSymbol& symbol = ite->first.second;
		if (symbol.SymbolType() != GrammarSymbolNonterminal) {
			continue;
		}

		int i = 0;
		Utility::ParseInteger(ite->first.first.ToString(), &i);

		int j = 0;
		Utility::ParseInteger(ite->second.ToString(), &j);
		gotoTable.insert(i, symbol, j);
	}

	return true;
}

void LALR::CalculateLR1Itemset(LR1Itemset& answer) {
	for (; CalculateLR1ItemsetOnePass(answer);) {
	}
}

bool LALR::GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	LR1EdgeTable::iterator pos = edges_.find(src.GetName(), symbol);
	if (pos != edges_.end()) {
		itemsets_.find(pos->second.ToString(), answer);
		return false;
	}

	bool newSetCreated = CalculateLR1EdgeTarget(answer, src, symbol);
	if (!answer.empty()) {
		edges_.insert(src.GetName(), symbol, answer.GetName());
		return newSetCreated;
	}

	return false;
}

bool LALR::CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	for (LR1Itemset::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		const Condinate* cond = grammars_->GetTargetCondinate(ite->cpos, nullptr);

		if (ite->dpos >= (int)cond->symbols.size() || cond->symbols[ite->dpos] != symbol) {
			continue;
		}

		LR1Item item(ite->cpos, ite->dpos + 1, ite->forwards);
		answer.insert(item);
	}

	CalculateLR1Itemset(answer);

	if (answer.empty()) {
		return false;
	}

	LR1ItemsetContainer::insert_status status = itemsets_.insert(answer);
	if (status.second) {
		answer.SetName(std::to_string(itemsets_.size() - 1));
		return true;
	}

	answer.SetName(status.first->GetName());
	return false;
}

bool LALR::CalculateLR1ItemsetOnePass(LR1Itemset& answer) {
	LR1Itemset newItems;
	for (LR1Itemset::iterator isi = answer.begin(); isi != answer.end(); ++isi) {
		const LR1Item& current = *isi;
		const Condinate* cond = grammars_->GetTargetCondinate(current.cpos, nullptr);

		if (current.dpos >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& b = cond->symbols[current.dpos];

		if (b.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		CalculateLR1ItemsetByLhs(newItems, current, b, cond);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::CalculateLR1ItemsetByLhs(LR1Itemset& answer, const LR1Item& item, const GrammarSymbol& lhs, const Condinate* cond) {
	int gi = 1;
	const CondinateContainer& conds = grammars_->FindGrammar(lhs, &gi)->GetCondinates();

	SymbolVector beta(cond->symbols.begin() + item.dpos + 1, cond->symbols.end());

	for (SymbolVector::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
		beta.push_back(*fi);

		GrammarSymbolSet firstSet;
		firstSetContainer_->GetFirstSet(firstSet, beta.begin(), beta.end());

		for (GrammarSymbolSet::iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
			if (fsi->SymbolType() == GrammarSymbolNonterminal) {
				continue;
			}

			int index = 0;
			for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++index) {
				LR1Item newItem(Utility::MakeDword(index, gi), 0);
				newItem.forwards.insert(*fsi);
				answer.insert(newItem);
			}
		}

		beta.pop_back();
	}
}

bool LALR::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			setChanged = GetLR1EdgeTarget(itemset, *ite, ite2->second) || setChanged;
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR1Itemset itemset;
			setChanged = GetLR1EdgeTarget(itemset, *ite, ite2->second) || setChanged;
		}
	}

	return setChanged;
}

void LALR::MergeItemsets() {
	LR1ItemsetContainer newItemsets;
	LR1ItemsetContainer::const_iterator first = itemsets_.begin(), pos;

	do {
		pos = Utility::FindGroup(first, itemsets_.end());
		LR1Itemset newSet;

		std::string name;
		char seperator = 0;
		for (LR1ItemsetContainer::const_iterator ite = first; ite != pos; ++ite) {
			if (seperator != 0) {
				name += seperator;
			}

			seperator = NAME_SEPERATOR;
			name += ite->GetName().ToString();
		}

		newSet.SetName(name);

		MergeNewItemset(newSet, first, pos);

		newItemsets.insert(newSet);

		first = pos;
	} while (first != itemsets_.end());

	LR1EdgeTable newEdges;
	RecalculateEdges(newEdges, newItemsets);
	NormalizeStateNames(newEdges, newItemsets);

	edges_ = newEdges;
	itemsets_ = newItemsets;
}

void LALR::NormalizeStateNames(LR1EdgeTable& newEdges, LR1ItemsetContainer& newItemsets) {
	Assert(newItemsets.begin()->GetName().ToString() == "0", "invalid start state position");
	int index = 1;
	for (LR1ItemsetContainer::iterator ite = newItemsets.begin(); (++ite) != newItemsets.end(); ) {
		LR1Itemset& item = (LR1Itemset&)*ite;
		item.SetName(std::to_string(index++));
	}
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

	Utility::Split(states, current.GetName().ToString(), NAME_SEPERATOR);

	for (vsiterator si = states.begin(); si != states.end(); ++si) {
		LR1Itemset source;
		itemsets_.find(*si, source);
		Assert(!source.empty(), "invalid source state");

		LR1ItemsetName target;
		if (!edges_.get(source.GetName(), symbol, target)) {
			continue;
		}

		FindMergeTarget(answer, target, newItemsets);
	}
}

void LALR::FindMergeTarget(LR1Itemset& answer, const LR1ItemsetName& name, const LR1ItemsetContainer& newItemsets) {
	// TODO:

	std::vector<std::string> states;
	for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
		Utility::Split(states, ite->GetName().ToString(), NAME_SEPERATOR);

		if (std::find(states.begin(), states.end(), name.ToString()) != states.end()) {
			answer = *ite;
			break;
		}

		states.clear();
	}
}

void LALR::MergeNewItemset(LR1Itemset &newSet, LR1ItemsetContainer::const_iterator first, LR1ItemsetContainer::const_iterator last) {
	for (int i = 0; i < first->size(); ++i) {
		LR1Itemset::iterator current = first->begin();
		std::advance(current, i);

		LR1Item newItem(current->cpos, current->dpos);

		for (LR1ItemsetContainer::const_iterator ite = first; ite != last; ++ite) {
			LR1Itemset::const_iterator ite2 = ite->begin();
			std::advance(ite2, i);

			for (Forwards::const_iterator svi = ite2->forwards.begin(); svi != ite2->forwards.end(); ++svi) {
				newItem.forwards.insert(*svi);
			}
		}

		newSet.insert(newItem);
	}
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
		tp << ite->GetName().ToString();
		LR1ItemsetName target;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			if (edges_.get(ite->GetName(), ite2->second, target)) {
				tp << target.ToString();
			}
			else {
				tp << "";
			}
		}

		for (GrammarSymbolContainer::const_iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			if (edges_.get(ite->GetName(), ite2->second, target)) {
				tp << target.ToString();
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
