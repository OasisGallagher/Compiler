#include <stack>
#include <sstream>
#include <algorithm>
#include "matrix.h"
#include "operator_precedence_parser.h"

typedef std::pair<GrammarSymbol, GrammarSymbol> SymbolPair;
typedef std::stack<SymbolPair> SymbolPairStack;

struct IsTerminalSymbol : std::unary_function<GrammarSymbol, bool> {
	bool operator() (const GrammarSymbol& symbol) const {
		return symbol.SymbolType() == GrammarSymbolTerminal;
	}
};

enum OperatorPrecedence {
	OperatorPrecedenceLess = -1,
	OperatorPrecedenceEqual,
	OperatorPrecedenceGreater,
};

class OperatorPrecedenceTable : public matrix <GrammarSymbol, GrammarSymbol, OperatorPrecedence> {
public:
	std::string ToString() const;
};

std::string OperatorPrecedenceTable::ToString() const {
	std::ostringstream oss;
	const char* strOperatorPrecedence[] = {
		"<",
		"=",
		">"
	};

	const char* seperator = "";
	for (const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << strOperatorPrecedence[value + 1];
	}

	return oss.str();
}

OperatorPrecedenceParser::OperatorPrecedenceParser() {
	operatorPrecedenceTable_ = new OperatorPrecedenceTable();
}

OperatorPrecedenceParser::~OperatorPrecedenceParser() {
	delete operatorPrecedenceTable_;
}

bool OperatorPrecedenceParser::ParseFile(SyntaxTree* tree, FileScanner* fileScanner) {
	return true;
}

std::string OperatorPrecedenceParser::ToString() const {
	std::ostringstream oss;
	oss << Parser::ToString();

	oss << "\n\n";

	oss << Utility::Heading(" First ") << "\n";
	oss << firstVtContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" Follow ") << "\n";
	oss << lastVtContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" OperatorPrecedenceTable ") << "\n";
	oss << operatorPrecedenceTable_->ToString();

	return oss.str();
}

bool OperatorPrecedenceParser::ParseGrammars() {
	Assert(IsOperatorGrammar, "invalid operator grammar");

	CreateFirstVt();
	CreateLastVt();
	CreateParsingTable();

	return true;
}

void OperatorPrecedenceParser::Clear() {
	firstVtContainer_.clear();
	lastVtContainer_.clear();
}

void OperatorPrecedenceParser::CreateFirstVt() {
	SymbolPairStack s;

	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			Condinate::iterator ntpos = std::find_if(c->begin(), c->end(), IsTerminalSymbol());
			if (ntpos == c->end()) {
				continue;
			}

			if (firstVtContainer_[g->GetLeft()].insert(*ntpos).second) {
				s.push(std::make_pair(g->GetLeft(), *ntpos));
			}
		}
	}

	for (; !s.empty(); ) {
		SymbolPair item = s.top();
		s.pop();

		for (GrammarContainer::const_iterator ite = grammars_.begin();
			ite != grammars_.end(); ++ite) {
			Grammar* g = *ite;
			const CondinateContainer& conds = g->GetCondinates();
			for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
				Condinate* c = *ite;
				if (c->front() != item.first) {
					continue;
				}

				if(firstVtContainer_[g->GetLeft()].insert(item.second).second) {
					s.push(std::make_pair(g->GetLeft(), item.second));
				}
			}
		}
	}
}

void OperatorPrecedenceParser::CreateLastVt() {
	// TODO: 流程与CreateFirstVt相似.
	SymbolPairStack s;

	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			Condinate::reverse_iterator ntpos = std::find_if(c->rbegin(), c->rend(), IsTerminalSymbol());
			if (ntpos == c->rend()) {
				continue;
			}

			if (lastVtContainer_[g->GetLeft()].insert(*ntpos).second) {
				s.push(std::make_pair(g->GetLeft(), *ntpos));
			}
		}
	}

	for (; !s.empty();) {
		SymbolPair item = s.top();
		s.pop();

		for (GrammarContainer::const_iterator ite = grammars_.begin();
			ite != grammars_.end(); ++ite) {
			Grammar* g = *ite;
			const CondinateContainer& conds = g->GetCondinates();
			for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
				Condinate* c = *ite;
				if (c->back() != item.first) {
					continue;
				}

				if (lastVtContainer_[g->GetLeft()].insert(item.second).second) {
					s.push(std::make_pair(g->GetLeft(), item.second));
				}
			}
		}
	}
}

void OperatorPrecedenceParser::CreateParsingTable() {
	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			BuildParsingTable(c);
		}
	}
}

void OperatorPrecedenceParser::BuildParsingTable(Condinate* c) {
	for (int i = 0; i < (int)c->size() - 1; ++i) {
		if (c->at(i).SymbolType() == GrammarSymbolTerminal && c->at(i + 1).SymbolType() == GrammarSymbolTerminal) {
			Assert(operatorPrecedenceTable_->find(c->at(i), c->at(i + 1)) == operatorPrecedenceTable_->end(), "invalid operator precedence grammar");
			operatorPrecedenceTable_->insert(c->at(i), c->at(i + 1), OperatorPrecedenceEqual);
		}

		if (i < (int)c->size() - 2
			&& c->at(i).SymbolType() == GrammarSymbolTerminal && c->at(i + 2).SymbolType() == GrammarSymbolTerminal && c->at(i + 1).SymbolType() == GrammarSymbolNonterminal) {
			Assert(operatorPrecedenceTable_->find(c->at(i), c->at(i + 2)) == operatorPrecedenceTable_->end(), "invalid operator precedence grammar");
			operatorPrecedenceTable_->insert(c->at(i), c->at(i + 2), OperatorPrecedenceEqual);
		}

		if (c->at(i).SymbolType() == GrammarSymbolTerminal && c->at(i + 1).SymbolType() == GrammarSymbolNonterminal) {
			GrammarSymbolSetTable::const_iterator pos = firstVtContainer_.find(c->at(i + 1));
			if (pos != firstVtContainer_.end()) {
				const GrammarSymbolSet& cont = pos->second;
				for (GrammarSymbolSet::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
					Assert(operatorPrecedenceTable_->find(c->at(i), *ite) == operatorPrecedenceTable_->end(), "invalid operator precedence grammar");
					operatorPrecedenceTable_->insert(c->at(i), *ite, OperatorPrecedenceLess);
				}
			}
		}

		if (c->at(i).SymbolType() == GrammarSymbolNonterminal && c->at(i + 1).SymbolType() == GrammarSymbolTerminal) {
			GrammarSymbolSetTable::const_iterator pos = lastVtContainer_.find(c->at(i));
			if (pos != lastVtContainer_.end()) {
				const GrammarSymbolSet& cont = pos->second;
				for (GrammarSymbolSet::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
					Assert(operatorPrecedenceTable_->find(*ite, c->at(i + 1)) == operatorPrecedenceTable_->end(), "invalid operator precedence grammar");
					operatorPrecedenceTable_->insert(*ite, c->at(i + 1), OperatorPrecedenceGreater);
				}
			}
		}
	}
}

bool OperatorPrecedenceParser::IsOperatorGrammar() const {
	struct GrammarSymbolTypeComparer : std::binary_function<GrammarSymbol, GrammarSymbol, bool > {
		bool operator ()(const GrammarSymbol& lhs, const GrammarSymbol& rhs) const {
			return lhs.SymbolType() == GrammarSymbolNonterminal && rhs.SymbolType() == GrammarSymbolNonterminal;
		}
	};

	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			if (std::adjacent_find(c->begin(), c->end(), GrammarSymbolTypeComparer()) != c->end()) {
				return false;
			}

			if (c->size() == 1 && c->front().SymbolType() == GrammarSymbolNonterminal) {
				Debug::LogWarning("production " + g->GetLeft().ToString() + " : " + c->ToString() + " will be omitted.");
			}
		}
	}

	return true;
}
