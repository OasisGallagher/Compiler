#include <sstream>

#include "lr0.h"
#include "table_printer.h"

LR0::LR0() {

}

LR0::~LR0() {
	for (LR0ClosureContainer::iterator ite = closures_.begin();
		ite != closures_.end(); ++ite) {
		delete ite->second;
	}
}

void LR0::Setup(const LRSetupParameter& parameter) {
	grammars_ = parameter.grammars;
	terminalSymbols_ = parameter.terminalSymbols;
	nonterminalSymbols_ = parameter.nonterminalSymbols;

	firstSetContainer_ = parameter.firstSetContainer;
	followSetContainer = parameter.followSetContainer;
}

bool LR0::Parse(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	if (!CreateLR0Itemsets()) {
		return false;
	}

	return CreateLRParsingTable(gotoTable, actionTable);
}

bool LR0::CreateLRParsingTable(LRGotoTable& gotoTable, LRActionTable& actionTable) {
	for (LR0Itemsets::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		LR0Closure* closure = *ite;
		for (LR0Closure::iterator ite2 = closure->begin(); ite2 != closure->end(); ++ite2) {
			const LR0Item& item = *ite2;
			const Condinate* cond = grammars_->GetTargetCondinate(item.cpos, nullptr);

			if (item.dpos >= (int)cond->symbols.size()) {
				continue;
			}

			const GrammarSymbol& symbol = cond->symbols[item.dpos];
		}
	}
	return true;
}

std::string LR0::ToString() const {
	std::ostringstream oss;

	oss << Utility::Heading(" LR0 Closures ") << "\n";
	oss << closures_.ToString(*grammars_);

	oss << "\n\n";

	oss << Utility::Heading(" LR0 Edges ") << "\n";
	oss << edges_.ToString(*grammars_);

	oss << "\n\n";

	oss << Utility::Heading(" LR0 Itemsets ") << "\n";
	oss << itemsets_.ToString(*grammars_);

	oss << "\n\n";

	oss << Utility::Heading(" LR0 DFA ") << "\n";
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
	for (LR0Itemsets::const_iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		tp << si++;
		LR0Closure* dest = nullptr;
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

bool LR0::CreateLR0Itemsets() {
	LR0Item start = { 0, 0 };
	itemsets_.insert(GetLR0Closure(start));

	for (; CreateLR0ItemsetsOnePass();) {

	}

	return true;
}

LR0Closure* LR0::GetLR0Closure(const LR0Item& item) {
	LR0ClosureContainer::iterator ite = closures_.find(item);
	if (ite != closures_.end()) {
		return ite->second;
	}

	LR0Closure* answer = CalculateLR0Closure(item);
	closures_.insert(std::make_pair(item, answer));
	return answer;
}

LR0Closure* LR0::CalculateLR0Closure(const LR0Item& item) {
	LR0Closure* answer = new LR0Closure();
	answer->insert(item);

	for (; CalculateLR0ClosureOnePass(answer);) {
	}

	return answer;
}

LR0Closure* LR0::GetLR0EdgeTarget(LR0Closure* src, const GrammarSymbol& symbol) {
	LR0EdgeTable::iterator ite = edges_.find(src, symbol);
	if (ite != edges_.end()) {
		return ite->second;
	}

	LR0Closure* answer = CalculateLR0EdgeTarget(src, symbol);
	if (answer != nullptr) {
		edges_.insert(src, symbol, answer);
	}

	return answer;
}

LR0Closure* LR0::CalculateLR0EdgeTarget(LR0Closure* src, const GrammarSymbol& symbol) {
	LR0Closure* answer = nullptr;
	for (LR0Closure::const_iterator ite = src->begin(); ite != src->end(); ++ite) {
		const Condinate* cond = grammars_->GetTargetCondinate(ite->cpos, nullptr);
		SymbolVector::const_iterator pos = std::find(cond->symbols.begin(), cond->symbols.end(), symbol);
		if (pos == cond->symbols.end() || ite->dpos != (int)std::distance(cond->symbols.begin(), pos)) {
			continue;
		}

		Assert(answer == nullptr, "closure is not unique");

		LR0Item item = { ite->cpos, ite->dpos + 1 };
		answer = GetLR0Closure(item);
	}

	return answer;
}

bool LR0::CalculateLR0ClosureOnePass(LR0Closure* answer) {
	bool setChanged = false;
	for (LR0Closure::iterator ite = answer->begin(); ite != answer->end(); ++ite) {
		const LR0Item& current = *ite;
		const Condinate* cond = grammars_->GetTargetCondinate(current.cpos, nullptr);
		if (current.dpos >= (int)cond->symbols.size()) {
			continue;
		}

		const GrammarSymbol& b = cond->symbols[current.dpos];

		if (b.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		int gi = 0;
		const CondinateContainer& conds = grammars_->FindGrammar(b, &gi)->GetCondinates();

		int index = 0;
		for (CondinateContainer::const_iterator ite2 = conds.begin(); ite2 != conds.end(); ++ite2, ++index) {
			LR0Item newItem = { Utility::MakeDword(index, gi), 0 };
			setChanged = answer->insert(newItem).second || setChanged;
		}
	}

	return setChanged;
}

bool LR0::CreateLR0ItemsetsOnePass() {
	bool setChanged = false;
	for (LR0Itemsets::iterator ite = itemsets_.begin(); ite != itemsets_.end(); ++ite) {
		for (GrammarSymbolContainer::iterator ite2 = terminalSymbols_->begin(); ite2 != terminalSymbols_->end(); ++ite2) {
			LR0Closure* target = GetLR0EdgeTarget(*ite, ite2->second);
			setChanged = (target != nullptr && itemsets_.insert(target).second) || setChanged;
		}

		for (GrammarSymbolContainer::iterator ite2 = nonterminalSymbols_->begin(); ite2 != nonterminalSymbols_->end(); ++ite2) {
			LR0Closure* target = GetLR0EdgeTarget(*ite, ite2->second);
			setChanged = (target != nullptr && itemsets_.insert(target).second) || setChanged;
		}
	}

	return setChanged;
}

bool LR0Item::operator < (const LR0Item& other) const {
	if (cpos == other.cpos) {
		return dpos < other.dpos;
	}

	return cpos < other.cpos;
}

std::string LR0Item::ToString(const GrammarContainer& grammars) const {
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

std::string LR0Closure::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = ", ";
		oss << "{ " << ite->ToString(grammars) << " }";
	}

	return oss.str();
}

std::string LR0Itemsets::ToString(const GrammarContainer& grammars) const {
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

std::string LR0ClosureContainer::ToString(const GrammarContainer& grammars) const {
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

std::string LR0EdgeTable::ToString(const GrammarContainer& grammars) const {
	std::ostringstream oss;

	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = "\n";

		LR0Closure* src = ite->first.first;
		const GrammarSymbol& symbol = ite->first.second;
		LR0Closure* dest = ite->second;

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
