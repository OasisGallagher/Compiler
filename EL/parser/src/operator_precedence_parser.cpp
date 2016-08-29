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
	const char* seperator = "";
	for (const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << value;
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
	Assert(IsOperatorGrammar(), "invalid operator grammar");

	CreateFirstVt();
	CreateLastVt();

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
					s.push(std::make_pair(g->GetLeft(), item.first));
				}
			}
		}
	}
}

void OperatorPrecedenceParser::CreateLastVt() {
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
					s.push(std::make_pair(g->GetLeft(), item.first));
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
		}
	}

	return true;
}
