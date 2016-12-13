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

	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LALR::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	return CreateActionTable(actionTable) && CreateGotoTable(gotoTable);
}

bool LALR::CreateLR1Itemsets() {
	LR1Item start(0, 0, new SymbolVector);
	start.forwards.insert(GrammarSymbol::zero);
	
	LR1Itemset itemset;
	itemset.SetName("0");

	itemset.insert(start);
	CalculateLR1Itemset(itemset);
	
	itemsets_.insert(itemset);

	for (; CreateLR1ItemsetsOnePass();) {

	}

	Debug::Log(itemsets_.ToString(*grammars_));

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
			int i = StateName2Integer(itemset.GetName());

			if (g->GetLhs() == GrammarSymbol::program && item.dpos == 1) {
				Assert(item.forwards.size() == 1 && *item.forwards.begin() == GrammarSymbol::zero, "invalid start state");
				LRAction action = { LRActionAccept };
				if (!actionTable.insert(i, GrammarSymbol::zero, action)) {
					Assert(false, "invalid lr grammar");
					return false;
				}

				continue;
			}

			if (item.dpos >= (int)cond->symbols.size()) {
				if (g->GetLhs() != GrammarSymbol::program) {
					LRAction action = { LRActionReduce, item.cpos };
					for (Forwards::const_iterator fi = item.forwards.begin(); fi != item.forwards.end(); ++fi) {
						if (!actionTable.insert(i, *fi, action)) {
							Assert(false, "invalid lr grammar");
							return false;
						}
					}
				}

				continue;
			}

			const GrammarSymbol& symbol = cond->symbols[item.dpos];
			if (symbol.SymbolType() != GrammarSymbolTerminal) {
				continue;
			}

			std::string target;
			if (edges_.get(itemset.GetName(), symbol, target)) {
				int j = StateName2Integer(target);
				LRAction action = { LRActionShift, j };
				if (!actionTable.insert(i, symbol, action)) {
					Assert(false, "invalid lr grammar");
					return false;
				}
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

		int i = StateName2Integer(ite->first.first);
		int j = StateName2Integer(ite->second);
		gotoTable.insert(i, symbol, j);
	}

	return true;
}

void LALR::CalculateLR1Itemset(LR1Itemset& answer) {
	for (; CalculateLR1ItemsetOnePass(answer);) {
	}
}

bool LALR::GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	//if (src.GetName() == "5" && symbol.ToString() == "c") {
		//__asm int 3
	//}

	LR1EdgeTable::iterator pos = edges_.find(src.GetName(), symbol);
	if (pos != edges_.end()) {
		itemsets_.find(pos->second, answer);
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

		LR1Item item(ite->cpos, ite->dpos + 1, new SymbolVector);// ite->forwards);
		for (Forwards::const_iterator fi = ite->forwards.begin(); fi != ite->forwards.end(); ++fi) {
			item.forwards.insert(*fi);
		}
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
	bool setChanged = false;
	for (LR1Itemset::iterator isi = answer.begin(); isi != answer.end(); ++isi) {
		const LR1Item& current = *isi;
		const Condinate* cond = grammars_->GetTargetCondinate(current.cpos, nullptr);
		std::string strCond = cond->ToString();

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

		for (SymbolVector::const_iterator fi = current.forwards.begin(); fi != current.forwards.end(); ++fi) {
			beta.push_back(*fi);

			GrammarSymbolSet firstSet;
			firstSetContainer_->GetFirstSet(firstSet, beta.begin(), beta.end());

			for (GrammarSymbolSet::iterator fsi = firstSet.begin(); fsi != firstSet.end(); ++fsi) {
				if (fsi->SymbolType() == GrammarSymbolNonterminal) {
					continue;
				}

				int index = 0;
				for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++index) {
					LR1Item newItem(Utility::MakeDword(index, gi), 0, new SymbolVector);
					newItem.forwards.insert(*fsi);

					setChanged = answer.insert(newItem) || setChanged;
				}
			}

			beta.pop_back();
		}
	}

	return setChanged;
}

bool LALR::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1ItemsetContainer::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		std::string name = ite->GetName();
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
			name += ite->GetName();
		}

		newSet.SetName(name);

		MergeNewItemset(newSet, first, pos);

		newItemsets.insert(newSet);

		first = pos;
	} while (first != itemsets_.end());

	LR1EdgeTable newEdges;
	RecalculateEdges(newEdges, newItemsets);

	edges_ = newEdges;
	itemsets_ = newItemsets;
}

void LALR::RecalculateEdges(LR1EdgeTable& newEdges, const LR1ItemsetContainer& newItemsets) {
	for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Itemset newTarget;
			if ((ite->GetName() == "5.1" || ite->GetName() == "1") && ite2->second.ToString() == "c") {
				//__asm int 3
			}
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

		std::string target;
		if (!edges_.get(source.GetName(), symbol, target)) {
			continue;
		}

		FindMergeTarget(answer, target, newItemsets);
	}
}

void LALR::FindMergeTarget(LR1Itemset& answer, const std::string& name, const LR1ItemsetContainer& newItemsets) {
	// TODO:
	std::vector<std::string> states;
	for (LR1ItemsetContainer::const_iterator ite = newItemsets.begin(); ite != newItemsets.end(); ++ite) {
		Utility::Split(states, ite->GetName(), NAME_SEPERATOR);

		if (std::find(states.begin(), states.end(), name) != states.end()) {
			answer = *ite;
			break;
		}
	}
}

void LALR::MergeNewItemset(LR1Itemset &newSet, LR1ItemsetContainer::const_iterator first, LR1ItemsetContainer::const_iterator last) {
	for (int i = 0; i < first->size(); ++i) {
		LR1Itemset::iterator current = first->begin();
		std::advance(current, i);

		LR1Item newItem(current->cpos, current->dpos, new SymbolVector);

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

int LALR::StateName2Integer(const std::string& name) {
	int answer = 0;
	for (const char* ptr = name.c_str(); *ptr != 0; ++ptr) {
		if (*ptr != NAME_SEPERATOR) {
			answer *= 10;
			answer += *ptr - '0';
		}
	}

	return answer;
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
		std::string target;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			edges_.get(ite->GetName(), ite2->second, target);
			tp << target;
		}

		for (GrammarSymbolContainer::const_iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			edges_.get(ite->GetName(), ite2->second, target);
			tp << target;
		}
	}

	tp.AddFooter();

	oss << tp.ToString();

	return oss.str();
}
