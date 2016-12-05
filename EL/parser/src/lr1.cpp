#include <sstream>

#include "lr1.h"
#include "table_printer.h"

LR1::LR1() {

}

LR1::~LR1() {
	for (LR1ClosureContainer::iterator ite = closures_.begin();
		ite != closures_.end(); ++ite) {
		delete ite->second;
	}
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
	for (LR1Itemsets::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		LR1Closure* closure = *ite;
		for (LR1Closure::iterator ite2 = closure->begin(); ite2 != closure->end(); ++ite2) {
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

	oss << Utility::Heading(" LR1 Closures ") << "\n";
	oss << closures_.ToString(*grammars_);

	oss << "\n\n";

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
	for (LR1Itemsets::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << si++;
		LR1Closure* dest = nullptr;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, dest)) {
				tp << (int)std::distance(itemsets_.begin(), std::find(itemsets_.begin(), itemsets_.end(), dest));
			}
			else {
				tp << "";
			}
		}

		for (GrammarSymbolContainer::const_iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			if (edges_.get(*ite, ite2->second, dest)) {
				tp << (int)std::distance(itemsets_.begin(), std::find(itemsets_.begin(), itemsets_.end(), dest));
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
	itemsets_.insert(GetLR1Closure(start));

	for (; CreateLR1ItemsetsOnePass();) {

	}

	return true;
}

LR1Closure* LR1::GetLR1Closure(const LR1Item& item) {
	LR1ClosureContainer::iterator ite = closures_.find(item);
	if (ite != closures_.end()) {
		return ite->second;
	}

	LR1Closure* answer = CalculateLR1Closure(item);
	closures_.insert(std::make_pair(item, answer));
	return answer;
}

LR1Closure* LR1::CalculateLR1Closure(const LR1Item& item) {
	LR1Closure* answer = new LR1Closure();
	answer->insert(item);

	for (; CalculateLR1ClosureOnePass(answer);) {
	}

	return answer;
}

LR1Closure* LR1::GetLR1EdgeTarget(LR1Closure* src, const GrammarSymbol& symbol) {
	LR1EdgeTable::iterator ite = edges_.find(src, symbol);
	if (ite != edges_.end()) {
		return ite->second;
	}

	LR1Closure* answer = CalculateLR1EdgeTarget(src, symbol);
	if (answer != nullptr) {
		edges_.insert(src, symbol, answer);
	}

	return answer;
}

LR1Closure* LR1::CalculateLR1EdgeTarget(LR1Closure* src, const GrammarSymbol& symbol) {
	LR1Closure* answer = nullptr;
	for (LR1Closure::const_iterator ite = src->begin(); ite != src->end(); ++ite) {
		const Condinate* cond = grammars_->GetTargetCondinate(ite->cpos, nullptr);
		SymbolVector::const_iterator pos = std::find(cond->symbols.begin(), cond->symbols.end(), symbol);
		if (pos == cond->symbols.end() || ite->dpos != (int)std::distance(cond->symbols.begin(), pos)) {
			continue;
		}

		Assert(answer == nullptr, "closure is not unique");

		LR1Item item = { ite->cpos, ite->dpos + 1, ite->forward };
		answer = GetLR1Closure(item);
	}

	return answer;
}

bool LR1::CalculateLR1ClosureOnePass(LR1Closure* answer) {
	bool setChanged = false;
	for (LR1Closure::iterator ite = answer->begin(); ite != answer->end(); ++ite) {
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
			}
		}
	}

	return setChanged;
}

bool LR1::CreateLR1ItemsetsOnePass() {
	bool setChanged = false;
	for (LR1Itemsets::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR1Closure* target = GetLR1EdgeTarget(*ite, ite2->second);
			setChanged = (target != nullptr && itemsets_.insert(target).second) || setChanged;
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR1Closure* target = GetLR1EdgeTarget(*ite, ite2->second);
			setChanged = (target != nullptr && itemsets_.insert(target).second) || setChanged;
		}
	}

	return setChanged;
}

bool LR1Item::operator < (const LR1Item& other) const {
	if (cpos == other.cpos) {
		return dpos < other.dpos;
	}

	return cpos < other.cpos;
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

	return oss.str();
}

std::string LR1Closure::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = ", ";
		oss << "{ " << ite->ToString(grammars) << " }";
	}

	return oss.str();
}

std::string LR1Itemsets::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	const char* seperator = "";
	int index = 0;
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";
		oss << "(" << index++ << ")\t" << " { " << (*ite)->ToString(grammars) << " }";
	}

	return oss.str();
}

std::string LR1ClosureContainer::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		oss << "{ " << ite->first.ToString(grammars) << " }";
		oss << " => ";
		oss << "{ " << (ite->second)->ToString(grammars) << " }";
	}

	return oss.str();
}

std::string LR1EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		LR1Closure* src = ite->first.first;
		const GrammarSymbol& symbol = ite->first.second;
		LR1Closure* dest = ite->second;

		oss << "( ";
		oss << src->ToString(grammars);
		oss << ", ";
		oss << symbol.ToString();
		oss << " )";

		oss << " => ";
		oss << "{ " << dest->ToString(grammars) << " }";
	}

	return oss.str();
}
