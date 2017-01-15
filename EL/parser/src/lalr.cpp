#include <sstream>
#include <algorithm>

#include "lr0.h"
#include "lalr.h"
#include "debug.h"
#include "define.h"
#include "grammar.h"
#include "table_printer.h"

LALR::LALR() {
}

LALR::~LALR() {
}

void LALR::Setup(const LRSetupParameter& parameter) {
	p_ = parameter;
}

bool LALR::Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	Debug::StartSample("create LR0 itemsets");

	LR0 lr0;
	lr0.Setup(p_);
	lr0.CreateLR0Itemsets(items_, itemsets_, edges_);
	
	Debug::StartSample("remove noncore items");
	for (LR1Itemset::const_iterator ite = items_.begin(); ite != items_.end(); ++ite) {
		itemDict_.insert(*ite);
	}

	items_.RemoveNoncoreItems();
	Debug::EndSample();

	Debug::EndSample();

	Debug::StartSample("calculate forwards and propagations");

	for (GrammarSymbolContainer::iterator ite = p_.terminalSymbols->begin();
		ite != p_.terminalSymbols->end(); ++ite) {
		Debug::StartSample("calculate for " + ite->second.ToString());
		CalculateForwardsAndPropagations(ite->second);
		Debug::EndSample();
	}

	for (GrammarSymbolContainer::iterator ite = p_.nonterminalSymbols->begin();
		ite != p_.nonterminalSymbols->end(); ++ite) {
		Debug::StartSample("calculate for " + ite->second.ToString());
		CalculateForwardsAndPropagations(ite->second);
		Debug::EndSample();
	}

	Debug::EndSample();

	FindItem(0, 0).GetForwards().insert(GrammarSymbol::zero, true);

	Debug::StartSample("propagate forwards");
	PropagateSymbols();
	Debug::EndSample();

	Debug::StartSample("create parsing table");
	bool status = CreateLRParsingTable(gotoTable, actionTable);
	Debug::EndSample();

	return status;
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	return CreateActionTable(actionTable) && CreateGotoTable(gotoTable);
}

bool LALR::CreateActionTable(LRActionTable &actionTable) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		const LR1Itemset& itemset = *ite;
		for (LR1Itemset::const_iterator ite2 = itemset.begin(); ite2 != itemset.end(); ++ite2) {
			const LR1Item& item = *ite2;
			ParseLRAction(actionTable, itemset, item);
		}
	}

	return true;
}

bool LALR::ParseLRAction(LRActionTable & actionTable, const LR1Itemset& itemset, const LR1Item &item) {
	Grammar* g = nullptr;
	const Condinate* cond = p_.grammars->GetTargetCondinate(item.GetCpos(), &g);
	int i = Utility::ParseInteger(itemset.GetName());

	if (g->GetLhs() == GrammarSymbol::program && item.GetDpos() == 1) {
		Assert(item.GetForwards().size() == 1 && item.GetForwards().begin()->symbol == GrammarSymbol::zero, "invalid start state");
		LRAction action = { LRActionAccept };
		return InsertActionTable(actionTable, i, GrammarSymbol::zero, action);
	}

	if (item.GetDpos() >= (int)cond->symbols.size()) {
		bool status = true;

		if (g->GetLhs() != GrammarSymbol::program) {
			LRAction action = { LRActionReduce, item.GetCpos() };
			for (Forwards::const_iterator fi = item.GetForwards().begin(); fi != item.GetForwards().end(); ++fi) {
				status = InsertActionTable(actionTable, i, fi->symbol, action) || status;
			}
		}

		return status;
	}

	const GrammarSymbol& symbol = cond->symbols[item.GetDpos()];
	if (symbol.SymbolType() == GrammarSymbolTerminal) {
		LR1Itemset target;
		if (edges_.get(itemset, symbol, target)) {
			int j = Utility::ParseInteger(target.GetName());
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
			int i = Utility::ParseInteger(ite->first.first.GetName());
			int j = Utility::ParseInteger(ite->second.GetName());
			gotoTable.insert(i, symbol, j);
		}
	}

	return true;
}

void LALR::CalculateLR1Itemset(LR1Itemset& answer) {
	for (; CalculateLR1ItemsetOnePass(answer);) {
	}
}

bool LALR::CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	LR1Itemset itemset = answer;
	for (LR1Itemset::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		const Condinate* cond = p_.grammars->GetTargetCondinate(ite->GetCpos(), nullptr);

		if (cond->symbols.front() == GrammarSymbol::epsilon) {
			continue;
		}

		if (ite->GetDpos() >= (int)cond->symbols.size() || cond->symbols[ite->GetDpos()] != symbol) {
			continue;
		}

		LR1Item item = FindItem(ite->GetCpos(), ite->GetDpos() + 1);
		const Forwards& forwards = ite->GetForwards();
		for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
			item.GetForwards().insert(fi->symbol, fi->spontaneous);
		}

		itemset.insert(item);
	}

	CalculateLR1Itemset(itemset);

	if (!itemset.empty()) {
		LR1ItemsetContainer::iterator pos = itemsets_.find(itemset);
		Assert(pos != itemsets_.end(), "can not find target itemset " + itemset.ToString(*p_.grammars));
		answer = *pos;
	}

	return !answer.empty();
}

bool LALR::CalculateLR1ItemsetOnePass(LR1Itemset& answer) {
	LR1Itemset newItems;
	for (LR1Itemset::iterator isi = answer.begin(); isi != answer.end(); ++isi) {
		const LR1Item& current = *isi;
		const Condinate* cond = p_.grammars->GetTargetCondinate(current.GetCpos(), nullptr);

		if (current.GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& lhs = cond->symbols[current.GetDpos()];

		if (lhs.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}
		
		CalculateLR1Items(newItems, current, cond, lhs);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::CalculateLR1Items(LR1Itemset& answer, const LR1Item& item, const Condinate* cond, const GrammarSymbol& lhs) {
	int gi = 1;
	Grammar* grammar = p_.grammars->FindGrammar(lhs, &gi);

	SymbolVector beta(cond->symbols.begin() + item.GetDpos() + 1, cond->symbols.end());
	beta.push_back(GrammarSymbol::null);

	GrammarSymbolSet firstSet;

	Forwards forwards = item.GetForwards();
	for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
		beta.back() = fi->symbol;
		p_.firstSetContainer->GetFirstSet(firstSet, beta.begin(), beta.end());

		for (GrammarSymbolSet::const_iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
			AddLR1Items(answer, *fsi, grammar, gi);
		}
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
			LR1Item newItem = FindItem(Utility::MakeDword(index, gi), dpos);
			newItem.GetForwards().insert(forward, true);
			answer.insert(newItem);

			if (*ite == GrammarSymbol::epsilon || !IsNullable(*ite))  {
				break;
			}
		}

		if (ite == tc->symbols.end()) {
			LR1Item newItem = FindItem(Utility::MakeDword(index, gi), dpos);
			newItem.GetForwards().insert(forward, true);
			answer.insert(newItem);
		}
	}
}

bool LALR::IsNullable(const GrammarSymbol& symbol) {
	GrammarSymbolSet& firsts = p_.firstSetContainer->at(symbol);
	return firsts.find(GrammarSymbol::epsilon) != firsts.end();
}

LR1Item LALR::FindItem(int cpos, int dpos) {
	tmp_.SetCpos(cpos);
	tmp_.SetDpos(dpos);
	LR1Itemset::iterator pos = itemDict_.find(tmp_);
	Assert(pos != itemDict_.end(), "can not find item.");
	return *pos;
}

std::string LALR::ToString() const {
	std::ostringstream oss;
	
	oss << Utility::Heading(" LR1 Edges ") << "\n";
	oss << edges_.ToString(*p_.grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 Itemsets ") << "\n";
	oss << itemsets_.ToString(*p_.grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 DFA ") << "\n";
	TablePrinter tp;

	int maxlength = 0;
	for (GrammarSymbolContainer::const_iterator ite = p_.terminalSymbols->begin();
		ite != p_.terminalSymbols->end(); ++ite) {
		if ((int)ite->first.length() > maxlength) {
			maxlength = ite->first.length();
		}
	} 

	tp.AddColumn("", maxlength + 2);
	for (GrammarSymbolContainer::const_iterator ite = p_.terminalSymbols->begin(); 
		ite != p_.terminalSymbols->end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	for (GrammarSymbolContainer::const_iterator ite = p_.nonterminalSymbols->begin();
		ite != p_.nonterminalSymbols->end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	tp.AddHeader();

	for (LR1ItemsetContainer::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << ite->GetName();
		LR1Itemset target;
		for (GrammarSymbolContainer::const_iterator ite2 = p_.terminalSymbols->begin(); 
			ite2 != p_.terminalSymbols->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, target)) {
				tp << target.GetName();
			}
			else {
				tp << "";
			}
		}

		for (GrammarSymbolContainer::const_iterator ite2 = p_.nonterminalSymbols->begin();
			ite2 != p_.nonterminalSymbols->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, target)) {
				tp << target.GetName();
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

void LALR::PropagateSymbols() {
	for (; PropagateSymbolsOnePass();) {
	}
}

bool LALR::PropagateSymbolsOnePass() {
	bool propagated = false;

	for (LR1Itemset::iterator ite = items_.begin(); ite != items_.end(); ++ite) {
		const LR1Item& src = *ite;
		if (!src.IsCore()) {
			continue;
		}

		propagated = PropagateFrom(src) || propagated;
	}

	return propagated;
}

bool LALR::PropagateFrom(const LR1Item &src) {
	Propagations::iterator pos = propagations_.find(src);
	if (pos == propagations_.end()) {
		return false;
	}

	bool propagated = false;
	const Forwards& forwards = src.GetForwards();
	
	LR1Itemset& itemset = (LR1Itemset&)pos->second;
	for (LR1Itemset::iterator is = itemset.begin(); is != itemset.end(); ++is) {
		LR1Item& target = (LR1Item&)*is;
		for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
			propagated = target.GetForwards().insert(fi->symbol, false) || propagated;
		}
	}

	return propagated;
}

void LALR::CalculateForwardsAndPropagations(const GrammarSymbol& symbol) {
	for (LR1Itemset::iterator ite = items_.begin(); ite != items_.end(); ++ite) {
		LR1Item item = *ite;
		if (!item.IsCore()) {
			continue;
		}

		item.GetForwards().insert(GrammarSymbol::unknown, true);

		LR1Itemset target;
		target.insert(item);

		CalculateLR1Itemset(target);

		AddForwardsAndPropagations(item, target, symbol);

		item.GetForwards().erase(GrammarSymbol::unknown);
	}
}

void LALR::AddForwardsAndPropagations(LR1Item& src, const LR1Itemset& itemset, const GrammarSymbol& symbol) {
	for (LR1Itemset::iterator ite = itemset.begin(); ite != itemset.end(); ++ite) {
		const Condinate* cond = p_.grammars->GetTargetCondinate(ite->GetCpos(), nullptr);
		if (ite->GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		if (cond->symbols.front() == GrammarSymbol::epsilon) {
			continue;
		}

		if (cond->symbols[ite->GetDpos()] != symbol) {
			continue;
		}

		LR1Item target = FindItem(ite->GetCpos(), ite->GetDpos() + 1);

		for (Forwards::const_iterator ite2 = ite->GetForwards().begin(); ite2 != ite->GetForwards().end(); ++ite2) {
			if (ite2->symbol == GrammarSymbol::unknown) {
				propagations_[src].insert(target);
			}
			else {
				target.GetForwards().insert(ite2->symbol, true);
			}
		}
	}
}
