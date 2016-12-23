#include <sstream>
#include <algorithm>

#include "lalr.h"
#include "debug.h"
#include "define.h"
#include "grammar.h"
#include "table_printer.h"

LALR::LALR() {
	builder_ = new LR1ItemsetBuilder;
}

LALR::~LALR() {
	delete builder_;
}

void LALR::Setup(const LRSetupParameter& parameter) {
	grammars_ = parameter.grammars;
	terminalSymbols_ = parameter.terminalSymbols;
	nonterminalSymbols_ = parameter.nonterminalSymbols;

	firstSetContainer_ = parameter.firstSetContainer;
	followSetContainer = parameter.followSetContainer;
}

bool LALR::Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	Debug::Log("Create itemsets.");
	if (!CreateLR1Itemsets()) {
		return false;
	}

	Debug::Log("Merge itemsets.");
	if (!MergeItemsets()) {
		return false;
	}

	Debug::Log("Create parsing table.");
	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	return CreateActionTable(actionTable) && CreateGotoTable(gotoTable);
}

bool LALR::CreateLR1Itemsets() {
	LR1Itemset itemset;
	AddLR1Items(itemset, GrammarSymbol::zero, grammars_->front(), 0);

	CalculateLR1Itemset(itemset);
	
	builder_->insert(itemset);

	for (; CreateLR1ItemsetsOnePass();) {
	}

	return true;
}

bool LALR::CreateActionTable(LRActionTable &actionTable) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		const LR1Itemset& itemset = *ite;
		for (LR1Itemset::const_iterator ite2 = itemset.begin(); ite2 != itemset.end(); ++ite2) {
			const LR1Item& item = *ite2;
			ParseLRAction(actionTable, itemset.GetName(), item);
		}
	}

	return true;
}

bool LALR::ParseLRAction(LRActionTable & actionTable, const LR1ItemsetName& name, const LR1Item &item) {
	Grammar* g = nullptr;
	const Condinate* cond = grammars_->GetTargetCondinate(item.cpos, &g);
	int i = Utility::ParseInteger(name.ToString());

	if (g->GetLhs() == GrammarSymbol::program && item.dpos == 1) {
		Assert(item.forwards.size() == 1 && *item.forwards.begin() == GrammarSymbol::zero, "invalid start state");
		LRAction action = { LRActionAccept };
		return InsertActionTable(actionTable, i, GrammarSymbol::zero, action);
	}

	if (item.dpos >= (int)cond->symbols.size()) {
		bool status = true;

		if (g->GetLhs() != GrammarSymbol::program) {
			LRAction action = { LRActionReduce, item.cpos };
			for (Forwards::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
				status = InsertActionTable(actionTable, i, *fi, action) || status;
			}
		}

		return status;
	}

	const GrammarSymbol& symbol = cond->symbols[item.dpos];
	if (symbol.SymbolType() == GrammarSymbolTerminal) {
		LR1ItemsetName target;
		if (edges_.get(name, symbol, target)) {
			int j = Utility::ParseInteger(target.ToString());
			LRAction action = { LRActionShift, j };
			return InsertActionTable(actionTable, i, symbol, action);
		}
	}

	return true;
}


bool LALR::CreateGotoTable(LRGotoTable &gotoTable) {
	for (LR1EdgeTable::const_iterator ite = edges_.begin(); ite != edges_.end(); ++ite) {
		const GrammarSymbol& symbol = ite->first.second;
		if (symbol.SymbolType() == GrammarSymbolNonterminal) {
			int i = Utility::ParseInteger(ite->first.first.ToString());
			int j = Utility::ParseInteger(ite->second.ToString());
			gotoTable.insert(i, symbol, j);
		}
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
		answer = builder_->operator[](pos->second);
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

		if (cond->symbols.front() == GrammarSymbol::epsilon) {
			continue;
		}

		if (ite->dpos >= (int)cond->symbols.size() || cond->symbols[ite->dpos] != symbol) {
			continue;
		}

		LR1Item item(ite->cpos, ite->dpos + 1, ite->forwards);
		std::string str = item.ToString(*grammars_);
		answer.insert(item);
	}

	CalculateLR1Itemset(answer);

	if (answer.empty()) {
		return false;
	}

	return builder_->insert(answer);
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

		CalculateLR1Items(newItems, current, b, cond);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::CalculateLR1Items(LR1Itemset& answer, const LR1Item& item, const GrammarSymbol& lhs, const Condinate* cond) {
	int gi = 1;
	Grammar* grammar = grammars_->FindGrammar(lhs, &gi);
	const CondinateContainer& conds = grammar->GetCondinates();

	SymbolVector beta(cond->symbols.begin() + item.dpos + 1, cond->symbols.end());

	GrammarSymbolSet firstSet;

	for (SymbolVector::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
		beta.push_back(*fi);
		firstSetContainer_->GetFirstSet(firstSet, beta.begin(), beta.end());
		beta.pop_back();

		for (GrammarSymbolSet::const_iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
			AddLR1Items(answer, *fsi, grammar, gi);
		}

		firstSet.clear();
	}
}

void LALR::AddLR1Items(LR1Itemset& answer, const GrammarSymbol& forward, Grammar* g, int gi) {
	int index = 0;
	const CondinateContainer& conds = g->GetCondinates();
	for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++index) {
		const Condinate* tc = *ci;
		SymbolVector::const_iterator ite = tc->symbols.begin();
		int dpos = 0;

		for (; ite != tc->symbols.end(); ++ite, ++dpos) {
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			newItem.forwards.insert(forward);
			answer.insert(newItem);

			if (*ite == GrammarSymbol::epsilon || !IsNullable(*ite))  {
				break;
			}
		}

		if (ite == tc->symbols.end()) {
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			newItem.forwards.insert(forward);
			answer.insert(newItem);
		}
	}
}

bool LALR::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1ItemsetContainer::iterator ite = builder_->begin(); ite != builder_->end(); ++ite) {
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

bool LALR::MergeItemsets() {
	ItemsetNameMap nameMap;
	LR1ItemsetContainer newItemsets;
	builder_->Merge(newItemsets, nameMap);

	LR1EdgeTable newEdges;
	RecalculateEdges(newEdges, newItemsets, nameMap);

	NormalizeStateNames(newItemsets);

	edges_.clear();
	edges_.insert(newEdges.begin(), newEdges.end());

	itemsets_ = newItemsets;

	delete builder_;
	builder_ = nullptr;

	return true;
}

void LALR::NormalizeStateNames(LR1ItemsetContainer& newItemsets) {
	Assert(newItemsets.begin()->GetName().ToString() == "0", "invalid start state position");
	int index = 1;
	for (LR1ItemsetContainer::iterator ite = newItemsets.begin(); (++ite) != newItemsets.end(); ) {
		LR1Itemset& item = (LR1Itemset&)*ite;
		item.SetName(std::to_string(index++));
	}
}

void LALR::RecalculateEdges(LR1EdgeTable& newEdges, LR1ItemsetContainer& newItemsets, ItemsetNameMap& nameMap) {
	for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			RecalculateNewEdgeTarget(newEdges, *ite, ite2->second, nameMap);
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			RecalculateNewEdgeTarget(newEdges, *ite, ite2->second, nameMap);
		}
	}
}

void LALR::RecalculateNewEdgeTarget(LR1EdgeTable& newEdges, const LR1Itemset& current, const GrammarSymbol& symbol, ItemsetNameMap& nameMap) {
	std::vector<std::string> states;
	typedef std::vector<std::string>::iterator vsiterator;

	Utility::Split(states, current.GetName().ToString(), ITEMSET_NAME_SEPERATOR);

	for (vsiterator si = states.begin(); si != states.end(); ++si) {
		LR1Itemset source = builder_->operator[](*si);

		Assert(!source.empty(), "invalid source state");

		LR1ItemsetName target;
		if (!edges_.get(source.GetName(), symbol, target)) {
			continue;
		}

		const LR1ItemsetName& newTarget = nameMap[target];
		Assert(!newTarget.empty(), "invalid new target");

		newEdges.insert(current.GetName(), symbol, newTarget);
	}
}

bool LALR::IsNullable(const GrammarSymbol& symbol) {
	GrammarSymbolSet& firsts = firstSetContainer_->at(symbol);
	return firsts.find(GrammarSymbol::epsilon) != firsts.end();
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
