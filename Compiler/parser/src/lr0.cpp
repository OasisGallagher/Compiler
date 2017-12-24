#include <sstream>

#include "lr0.h"
#include "debug.h"
#include "parser.h"
#include "table_printer.h"

LR0::LR0() {
}

LR0::~LR0() {
}

void LR0::Setup(Environment* env, FirstSetTable* firstSets) {
	env_ = env;
	firstSets_ = firstSets;
}

bool LR0::CreateLR0Itemsets(LR1ItemsetContainer& itemsets, LR1EdgeTable& edges) {
	LR1Itemset itemset;
	AddLR1Items(itemset, env_->grammars.front()->GetLhs());
	itemset.SetName("0");
	CalculateClosure(itemset);
	itemsets_.insert(itemset);

	for (; CreateLR1ItemsetsOnePass();) {
	}

	edges = edges_;
	itemsets = itemsets_;

	return true;
}

bool LR0::CreateLR1ItemsetsOnePass() {
	static int pass = 0;
	Debug::StartSample(Utility::Format("CreateLR1Itemsets (%d: %d)", ++pass, itemsets_.size()));
	bool setChanged = false;
	LR1ItemsetContainer cont = itemsets_;
	for (LR1ItemsetContainer::iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = env_->terminalSymbols.begin(); ite2 != env_->terminalSymbols.end(); ++ite2) {
			LR1Itemset itemset;
			setChanged = GetLR1EdgeTarget(itemset, *ite, ite2->second) || setChanged;
		}

		for (GrammarSymbolContainer::iterator ite2 = env_->nonterminalSymbols.begin(); ite2 != env_->nonterminalSymbols.end(); ++ite2) {
			if (ite2->second == NativeSymbols::program) {
				continue;
			}

			LR1Itemset itemset;
			setChanged = GetLR1EdgeTarget(itemset, *ite, ite2->second) || setChanged;
		}
	}

	Debug::EndSample();

	return setChanged;
}

void LR0::AddLR1Items(LR1Itemset& answer, const GrammarSymbol& lhs) {
	int index = 0, gi = 0;
	Grammar* g = env_->grammars.FindGrammar(lhs, &gi);
	const CondinateContainer& conds = g->GetCondinates();

	for (CondinateContainer::const_iterator ci = conds.begin(); ci != conds.end(); ++ci, ++index) {
		const Condinate* tc = *ci;
		SymbolVector::const_iterator ite = tc->symbols.begin();
		int dpos = 0;

		for (; ite != tc->symbols.end(); ++ite, ++dpos) {
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			answer.insert(newItem);

			if (*ite == NativeSymbols::epsilon || !IsNullable(*ite)) {
				break;
			}
		}

		if (ite == tc->symbols.end()) {
			LR1Item newItem(Utility::MakeDword(index, gi), dpos);
			answer.insert(newItem);
		}
	}
}

bool LR0::IsNullable(const GrammarSymbol& symbol) {
	GrammarSymbolSet& firsts = firstSets_->at(symbol);
	return firsts.find(NativeSymbols::epsilon) != firsts.end();
}

void LR0::CalculateClosure(LR1Itemset& answer) {
	for (; CalculateClosureOnePass(answer);) {
	}
}

bool LR0::CalculateClosureOnePass(LR1Itemset& answer) {
	LR1Itemset newItems;
	for (LR1Itemset::iterator isi = answer.begin(); isi != answer.end(); ++isi) {
		const LR1Item& current = *isi;
		const Condinate* cond = env_->grammars.GetTargetCondinate(current.GetCpos(), nullptr);

		if (current.GetDpos() >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& b = cond->symbols[current.GetDpos()];

		if (b.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		AddLR1Items(newItems, b);
	}

	bool setChanged = false;
	for (LR1Itemset::const_iterator ite = newItems.begin(); ite != newItems.end(); ++ite) {
		setChanged = answer.insert(*ite) || setChanged;
	}

	return setChanged;
}

bool LR0::GetLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	LR1EdgeTable::iterator pos = edges_.find(src, symbol);
	if (pos != edges_.end()) {
		answer = pos->second;
		return false;
	}

	bool newSetCreated = CalculateLR1EdgeTarget(answer, src, symbol);
	if (!answer.empty()) {
		edges_.insert(src, symbol, answer);
		return newSetCreated;
	}

	return false;
}

bool LR0::CalculateLR1EdgeTarget(LR1Itemset& answer, const LR1Itemset& src, const GrammarSymbol& symbol) {
	for (LR1Itemset::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		const Condinate* cond = env_->grammars.GetTargetCondinate(ite->GetCpos(), nullptr);

		if (cond->symbols.front() == NativeSymbols::epsilon) {
			continue;
		}

		if (ite->GetDpos() >= (int)cond->symbols.size() || cond->symbols[ite->GetDpos()] != symbol) {
			continue;
		}

		LR1Item item(ite->GetCpos(), ite->GetDpos() + 1);
		answer.insert(item);
	}

	if (answer.empty()) {
		return false;
	}

	CalculateClosure(answer);

	std::pair<LR1ItemsetContainer::iterator, bool> status = itemsets_.insert(answer);
	if (answer.GetName().empty()) {
		if (status.second) {
			answer.SetName(std::to_string(itemsets_.size() - 1));
		}
		else {
			answer = *status.first;
		}
	}

	return status.second;
}
