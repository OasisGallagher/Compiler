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
	LR0 lr0;
	lr0.Setup(p_);
	lr0.CreateLR0Itemsets(itemsets_, edges_);

	for (GrammarSymbolContainer::iterator ite = p_.terminalSymbols->begin();
		ite != p_.terminalSymbols->end(); ++ite) {
		CalculateForwardsAndPropagations(ite->second);
	}

	for (GrammarSymbolContainer::iterator ite = p_.nonterminalSymbols->begin();
		ite != p_.nonterminalSymbols->end(); ++ite) {
		CalculateForwardsAndPropagations(ite->second);
	}

	Debug::Log("Create parsing table.");
	return true;// CreateLRParsingTable(gotoTable, actionTable);
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
	const Condinate* cond = p_.grammars->GetTargetCondinate(item.cpos, &g);
	int i = Utility::ParseInteger(itemset.GetName());

	if (g->GetLhs() == GrammarSymbol::program && item.dpos == 1) {
		Assert(item.forwards.size() == 1 && item.forwards.begin()->symbol == GrammarSymbol::zero, "invalid start state");
		LRAction action = { LRActionAccept };
		return InsertActionTable(actionTable, i, GrammarSymbol::zero, action);
	}

	if (item.dpos >= (int)cond->symbols.size()) {
		bool status = true;

		if (g->GetLhs() != GrammarSymbol::program) {
			LRAction action = { LRActionReduce, item.cpos };
			for (Forwards::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
				status = InsertActionTable(actionTable, i, fi->symbol, action) || status;
			}
		}

		return status;
	}

	const GrammarSymbol& symbol = cond->symbols[item.dpos];
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
		const Condinate* cond = p_.grammars->GetTargetCondinate(ite->cpos, nullptr);

		if (cond->symbols.front() == GrammarSymbol::epsilon) {
			continue;
		}

		if (ite->dpos >= (int)cond->symbols.size() || cond->symbols[ite->dpos] != symbol) {
			continue;
		}

		LR1Item item(ite->cpos, ite->dpos + 1, ite->forwards);
		itemset.insert(item);
	}

	CalculateLR1Itemset(itemset);
	itemset.RemoveNoncoreItems();

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
		const Condinate* cond = p_.grammars->GetTargetCondinate(current.cpos, nullptr);

		if (current.dpos >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& b = cond->symbols[current.dpos];

		if (b.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}
		
		CalculateLR1Items(newItems, current);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::CalculateLR1Items(LR1Itemset& answer, const LR1Item& item) {
	const Condinate* cond = p_.grammars->GetTargetCondinate(item.cpos, nullptr);
	const GrammarSymbol& lhs = cond->symbols[item.dpos];

	int gi = 1;
	Grammar* grammar = p_.grammars->FindGrammar(lhs, &gi);
	const CondinateContainer& conds = grammar->GetCondinates();

	SymbolVector beta(cond->symbols.begin() + item.dpos + 1, cond->symbols.end());
	beta.push_back(GrammarSymbol::null);

	GrammarSymbolSet firstSet;

	for (Forwards::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
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
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			newItem.forwards.insert(forward, true);
			answer.insert(newItem);

			if (*ite == GrammarSymbol::epsilon || !IsNullable(*ite))  {
				break;
			}
		}

		if (ite == tc->symbols.end()) {
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			newItem.forwards.insert(forward, true);
			answer.insert(newItem);
		}
	}
}

bool LALR::IsNullable(const GrammarSymbol& symbol) {
	GrammarSymbolSet& firsts = p_.firstSetContainer->at(symbol);
	return firsts.find(GrammarSymbol::epsilon) != firsts.end();
}

std::string LALR::ToString() const {
	std::ostringstream oss;
	
	oss << Utility::Heading(" LR1 Edges ") << "\n";
	oss << edges_.ToString(*p_.grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 Itemsets ") << "\n";
	oss << itemsets_.ToString(*p_.grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 Propagations ") << "\n";
	oss << propagations_.ToString(*p_.grammars);

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

void LALR::CalculateForwardsAndPropagations(const GrammarSymbol& symbol) {
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (LR1Itemset::iterator ite2 = ite->begin(); ite2 != ite->end(); ++ite2) {
			LR1Item item = *ite2;
			item.forwards.insert(GrammarSymbol::unknown, true);

			LR1Itemset target;
			target.insert(item);
			target.SetName(ite->GetName());
			CalculateLR1Itemset(target);
			//CalculateLR1EdgeTarget(target, src, symbol);

			AddForwardsAndPropagations((LR1Item&)*ite2, *ite, target);
		}
	}
}

void LALR::AddForwardsAndPropagations(LR1Item& item, const LR1Itemset& src, const LR1Itemset& itemset) {
	for (LR1Itemset::iterator ite = itemset.begin(); ite != itemset.end(); ++ite) {
		const Condinate* cond = p_.grammars->GetTargetCondinate(item.cpos, nullptr);
		if (ite->dpos >= (int)cond->symbols.size()) {
			continue;
		}

		LR1Itemset parent;
		LR1Item target(ite->cpos, ite->dpos + 1);
		LR1ItemsetContainer::iterator pos = itemsets_.begin();

		for (; pos != itemsets_.end(); ++pos) {
			if (pos->find(target) != pos->end()) {
				parent = *pos;
				break;
			}
		}

		Assert(pos != itemsets_.end(), "can not find src itemset " + ite->ToString(*p_.grammars));

		for (Forwards::const_iterator ite2 = ite->forwards.begin(); ite2 != ite->forwards.end(); ++ite2) {
			if (ite2->symbol == GrammarSymbol::unknown) {
				propagations_[src].insert(parent);
			}
			else {
				item.forwards.insert(ite2->symbol, true);
			}
		}
	}
}
