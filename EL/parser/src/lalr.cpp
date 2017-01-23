#include <sstream>
#include <algorithm>

#include "lr0.h"
#include "lalr.h"
#include "debug.h"
#include "define.h"
#include "grammar.h"
#include "lr_table.h"
#include "table_printer.h"

LALR::LALR() :coreItemsCount_(0){
}

LALR::~LALR() {
}

void LALR::Setup(Environment* env, FirstSetTable* firstSets) {
	env_ = env;
	firstSets_ = firstSets;
}

bool LALR::Parse(LRActionTable& actionTable, LRGotoTable& gotoTable) {
	Debug::StartSample("create LR0 itemsets");
	LR0 lr0;
	lr0.Setup(env_, firstSets_);
	lr0.CreateLR0Itemsets(itemsets_, edges_);
	Debug::EndSample();
	
	coreItemsCount_ = 0;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		coreItemsCount_ += std::count_if(ite->begin(), ite->end(), std::mem_fun_ref(&LR1Item::IsCore));
	}

	Debug::Log(itemsets_.ToString(env_->grammars));

	Debug::StartSample("calculate forwards and propagations");
	CalculateForwardsAndPropagations();
	Debug::Log(itemsets_.ToString(env_->grammars));
	Debug::Log(propagations_.ToString(env_->grammars));
	Debug::EndSample();

	LR1Item init = *itemsets_.begin()->begin();
	init.GetForwards().insert(NativeSymbols::zero, true);

	Debug::StartSample("propagate forwards");
	PropagateSymbols();
	Debug::EndSample();

	Debug::Log(itemsets_.ToString(env_->grammars));

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
	const Condinate* cond = env_->grammars.GetTargetCondinate(item.GetCpos(), &g);
	int i = Utility::ParseInteger(itemset.GetName());

	if (g->GetLhs() == NativeSymbols::program && item.GetDpos() == 1 
		&& item.GetForwards().size() == 1 && item.GetForwards().begin()->symbol == NativeSymbols::zero) {
		LRAction action = { LRActionAccept };
		return InsertActionTable(actionTable, i, NativeSymbols::zero, action);
	}

	if (item.GetDpos() >= (int)cond->symbols.size()) {
		bool status = true;

		if (g->GetLhs() != NativeSymbols::program) {
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

void LALR::CalculateLR1Itemset(LR1Itemset& answer, LR1Itemset& itemset) {
	for (; CalculateLR1ItemsetOnePass(answer, itemset);) {
	}
}

bool LALR::CalculateLR1ItemsetOnePass(LR1Itemset& answer, LR1Itemset& itemset) {
	LR1Itemset newItems;

	for (LR1Itemset::iterator isi = answer.begin(); isi != answer.end(); ++isi) {
		const LR1Item& current = *isi;
		const Condinate* cond = env_->grammars.GetTargetCondinate(current.GetCpos(), nullptr);

		if (current.GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& lhs = cond->symbols[current.GetDpos()];

		if (lhs.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		AddLR1Items(newItems, lhs, current, itemset);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

void LALR::AddLR1Items(LR1Itemset &answer, const GrammarSymbol& lhs, const LR1Item &current, LR1Itemset& itemset) {
	int gi = 1;
	GrammarSymbolSet firstSet;
	
	Grammar* grammar = env_->grammars.FindGrammar(lhs, &gi);
	const Condinate* srcCond = env_->grammars.GetTargetCondinate(current.GetCpos(), nullptr);

	SymbolVector beta(srcCond->symbols.begin() + current.GetDpos() + 1, srcCond->symbols.end());
	beta.push_back(NativeSymbols::null);

	int condinateIndex = 0;
	const CondinateContainer& conds = grammar->GetCondinates();

	for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++condinateIndex) {
		const Condinate* tc = *ci;
		Forwards forwards = current.GetForwards();
		for (Forwards::const_iterator fi = forwards.begin(); fi != forwards.end(); ++fi) {
			beta.back() = fi->symbol;
			firstSets_->GetFirstSet(firstSet, beta.begin(), beta.end());

			for (GrammarSymbolSet::const_iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
				SymbolVector::const_iterator ite = tc->symbols.begin();
				int dpos = 0;

				for (; ite != tc->symbols.end(); ++ite, ++dpos) {
					LR1Item newItem = FindItem(Utility::MakeDword(condinateIndex, gi), dpos, itemset);
					newItem.GetForwards().insert(*fsi, true);
					answer.insert(newItem);

					if (*ite == NativeSymbols::epsilon || !IsNullable(*ite)) {
						break;
					}
				}

				if (ite == tc->symbols.end()) {
					LR1Item newItem = FindItem(Utility::MakeDword(condinateIndex, gi), dpos, itemset);
					newItem.GetForwards().insert(*fsi, true);
					answer.insert(newItem);
				}
			}

			firstSet.clear();
		}
	}
}

bool LALR::IsNullable(const GrammarSymbol& symbol) {
	GrammarSymbolSet& firsts = firstSets_->at(symbol);
	return firsts.find(NativeSymbols::epsilon) != firsts.end();
}

LR1Item LALR::FindItem(int cpos, int dpos, LR1Itemset& dict) {
	tmp_.SetCpos(cpos);
	tmp_.SetDpos(dpos);
	LR1Itemset::iterator pos = dict.find(tmp_);
	Assert(pos != dict.end(), "can not find item.");
	return *pos;
}

std::string LALR::ToString() const {
	std::ostringstream oss;
	
	oss << Utility::Heading(" LR1 Edges ") << "\n";
	oss << edges_.ToString(env_->grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 Itemsets ") << "\n";
	oss << itemsets_.ToString(env_->grammars);

	oss << "\n\n";

	oss << Utility::Heading(" LR1 DFA ") << "\n";
	TablePrinter tp;

	int maxlength = 0;
	for (GrammarSymbolContainer::const_iterator ite = env_->terminalSymbols.begin();
		ite != env_->terminalSymbols.end(); ++ite) {
		if ((int)ite->first.length() > maxlength) {
			maxlength = ite->first.length();
		}
	} 

	tp.AddColumn("", maxlength + 2);
	for (GrammarSymbolContainer::const_iterator ite = env_->terminalSymbols.begin();
		ite != env_->terminalSymbols.end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	for (GrammarSymbolContainer::const_iterator ite = env_->nonterminalSymbols.begin();
		ite != env_->nonterminalSymbols.end(); ++ite) {
		tp.AddColumn(ite->second.ToString(), ite->second.ToString().length() + 2);
	}

	tp.AddHeader();

	for (LR1ItemsetContainer::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << ite->GetName();
		LR1Itemset target;
		for (GrammarSymbolContainer::const_iterator ite2 = env_->terminalSymbols.begin(); 
			ite2 != env_->terminalSymbols.end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, target)) {
				tp << target.GetName();
			}
			else {
				tp << "";
			}
		}

		for (GrammarSymbolContainer::const_iterator ite2 = env_->nonterminalSymbols.begin();
			ite2 != env_->nonterminalSymbols.end(); ++ite2) {
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

	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (LR1Itemset::iterator ii = ite->begin(); ii != ite->end(); ++ii) {
			propagated = PropagateFrom(*ii) || propagated;
		}
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

void LALR::CalculateForwardsAndPropagations() {
	Debug::StartSample("add forwards and propagations");
	Debug::StartProgress();
	int index = 1;
	LR1Itemset target = nullptr;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		LR1Itemset& dict = (LR1Itemset&)*ite;
		
		for (LR1Itemset::iterator ii = dict.begin(); ii != dict.end(); ++ii) {
			LR1Item item = *ii;
			if (!item.IsCore()) {
				continue;
			}

			Debug::LogProgress("progress", index, coreItemsCount_);
			LR1Itemset itemset;

			item.GetForwards().insert(NativeSymbols::unknown, true);
			itemset.insert(item);

			CalculateLR1Itemset(itemset, dict);

			GrammarSymbolContainer::iterator si = env_->terminalSymbols.begin();
			for (; si != env_->terminalSymbols.end(); ++si) {
				if (edges_.get(dict, si->second, target)) {
					AddForwardsAndPropagations(item, itemset, target, si->second);
				}
			}

			si = env_->nonterminalSymbols.begin();
			for (; si != env_->nonterminalSymbols.end(); ++si) {
				if (si->second != NativeSymbols::program && edges_.get(dict, si->second, target)) {
					AddForwardsAndPropagations(item, itemset, target, si->second);
				}
			}

			++index;
		}
	}

	Debug::EndProgress();
	Debug::EndSample();

	Debug::StartSample("clean up");
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (LR1Itemset::iterator ii = ite->begin(); ii != ite->end(); ++ii) {
			Forwards& forwards = (Forwards&)ii->GetForwards();
			forwards.erase(NativeSymbols::unknown);
		}
	}

	Debug::EndSample();
}

void LALR::AddForwardsAndPropagations(LR1Item& src, const LR1Itemset& itemset, LR1Itemset& dict, const GrammarSymbol& symbol) {
	for (LR1Itemset::iterator ite = itemset.begin(); ite != itemset.end(); ++ite) {
		const Condinate* cond = env_->grammars.GetTargetCondinate(ite->GetCpos(), nullptr);
		if (ite->GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		if (cond->symbols.front() == NativeSymbols::epsilon) {
			continue;
		}

		if (cond->symbols[ite->GetDpos()] != symbol) {
			continue;
		}

		LR1Item target = FindItem(ite->GetCpos(), ite->GetDpos() + 1, dict);
		const Forwards& forwards = ite->GetForwards();
		for (Forwards::const_iterator ite2 = forwards.begin(); ite2 != forwards.end(); ++ite2) {
			if (ite2->symbol == NativeSymbols::unknown) {
				propagations_[src].insert(target);
			}
			else {
				target.GetForwards().insert(ite2->symbol, true);
			}
		}
	}
}
