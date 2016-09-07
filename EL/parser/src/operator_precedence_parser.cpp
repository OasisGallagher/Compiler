#include <stack>
#include <sstream>
#include <algorithm>

#include "matrix.h"
#include "scanner.h"
#include "table_printer.h"
#include "disjoint_sets.h"
#include "operator_precedence_parser.h"

using OperatorPrecedence = OperatorPrecedenceParser::OperatorPrecedence;

typedef std::pair<GrammarSymbol, GrammarSymbol> SymbolPair;
typedef std::stack<SymbolPair> SymbolPairStack;

static const char* strOperatorPrecedence[] = {
	"",
	"<",
	"=",
	">"
};

#define INSERT_OPERATOR_PRECEDENCE(_Table, _K1, _K2, _Val) \
	do { \
		OperatorPrecedenceTable::const_iterator pos = _Table->find(_K1, _K2); \
		if (pos == _Table->end()) { \
			_Table->insert(_K1, _K2, _Val); \
		}\
		else if(_Val != pos->second) { \
			Debug::LogWarning(std::string("invalid operator precedence grammar, precedence (") \
				+ (_K1).ToString() + ", " + (_K2).ToString() + ") : (" + strOperatorPrecedence[pos->second] + ", " + strOperatorPrecedence[_Val] + ")"); \
		}\
	} while (0)

struct IsTerminalSymbol : std::unary_function<GrammarSymbol, bool> {
	bool operator() (const GrammarSymbol& symbol) const {
		return symbol.SymbolType() == GrammarSymbolTerminal;
	}
};

class SymbolStack : public std::vector<GrammarSymbol> {
};

class OperatorPrecedenceTable : public matrix <GrammarSymbol, GrammarSymbol, OperatorPrecedence> {
public:
	std::string ToString(const GrammarSymbolContainer& terminalSymbols) const;
};

std::string OperatorPrecedenceTable::ToString(const GrammarSymbolContainer& terminalSymbols) const {
	std::ostringstream oss;
	/*const char* seperator = "";
	for (const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << strOperatorPrecedence[value];
	}

	oss << "\n\n";
	*/
	TablePrinter tp;

	int maxlength = 0;
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols.begin(); ite != terminalSymbols.end(); ++ite) {
		if ((int)ite->first.length() > maxlength) {
			maxlength = ite->first.length();
		}
	}

	tp.AddColumn("", maxlength + 2);

	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols.begin(); ite != terminalSymbols.end(); ++ite) {
		tp.AddColumn(ite->first, ite->first.length() + 2);
	}

	tp.AddHeader();
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols.begin(); ite != terminalSymbols.end(); ++ite) {
		tp << ite->first;
		for (GrammarSymbolContainer::const_iterator ite2 = terminalSymbols.begin(); ite2 != terminalSymbols.end(); ++ite2) {
			OperatorPrecedenceTable::const_iterator pos = find(ite->second, ite2->second);
			tp << ((pos == end()) ? "" : strOperatorPrecedence[pos->second]);
		}
	}

	tp.AddFooter();

	oss << tp.ToString();

	return oss.str();
}

struct CreateVtHelper {
	static bool FindFirstTerminalSymbol(GrammarSymbol& symbol, const Condinate* c) {
		Condinate::const_iterator ntpos = std::find_if(c->begin(), c->end(), IsTerminalSymbol());
		if (ntpos == c->end()) {
			return false;
		}

		symbol = *ntpos;
		return true;
	}

	static GrammarSymbol Front(const Condinate* c) {
		return c->front();
	}

	static bool FindLastTerminalSymbol(GrammarSymbol& symbol, const Condinate* c) {
		Condinate::const_reverse_iterator ntpos = std::find_if(c->rbegin(), c->rend(), IsTerminalSymbol());
		if (ntpos == c->rend()) {
			return false;
		}

		symbol = *ntpos;
		return true;
	}

	static GrammarSymbol Back(const Condinate* c) {
		return c->back();
	}
};

OperatorPrecedenceParser::OperatorPrecedenceParser() {
	operatorPrecedenceTable_ = new OperatorPrecedenceTable();
}

OperatorPrecedenceParser::~OperatorPrecedenceParser() {
	delete operatorPrecedenceTable_;
}

bool OperatorPrecedenceParser::ComparePrecedence(const GrammarSymbol& lhs, const GrammarSymbol& rhs, OperatorPrecedence precedence) const {
	OperatorPrecedenceTable::const_iterator pos = operatorPrecedenceTable_->find(lhs, rhs);

	if (pos == operatorPrecedenceTable_->end()) {
		return false;
	}

	return pos->second == precedence;
}

template <class Iterator>
bool OperatorPrecedenceParser::MatchProduction(const Condinate* c, Iterator first, Iterator last) const {
	Condinate::const_iterator pos = c->begin();
	for (; first != last && pos != c->end(); ++first, ++pos) {
		if (first->SymbolType() != pos->SymbolType()) {
			return false;
		}

		if (first->SymbolType() == GrammarSymbolTerminal && *first != *pos) {
			return false;
		}
	}

	return (first == last) && (pos == c->end());
}

template <class Iterator>
GrammarSymbol OperatorPrecedenceParser::Reduce(Iterator first, Iterator last) {
	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			if (MatchProduction(c, first, last)) {
				return g->GetLhs();
			}
		}
	}

	return GrammarSymbol::null;
}

bool OperatorPrecedenceParser::ParseFile(SyntaxTree* tree, FileScanner* fileScanner) {
	SymbolStack container;
	container.push_back(GrammarSymbol::zero);

	int k = 0;

	ScannerToken token;
	TokenPosition position = { 0 };

	int reduceCount = 0;

	GrammarSymbol a;
	do {
		a = GrammarSymbol::zero;
		if (fileScanner->GetToken(&token, &position)) {
			a = FindSymbol(token);
		}

		if (!a) {
			Debug::LogError("invalid token at " + position.ToString());
			return false;
		}

		int j = (container[k].SymbolType() == GrammarSymbolTerminal) ? k : k - 1;
		OperatorPrecedence precedence = OperatorPrecedenceEmpty;

		for (; ComparePrecedence(container[j], a, OperatorPrecedenceGreater);) {
			GrammarSymbol q;
			do {
				q = container[j];
				j -= (container[j - 1].SymbolType() == GrammarSymbolTerminal) ? 1 : 2;

			} while (!ComparePrecedence(container[j], q, OperatorPrecedenceLess));

			GrammarSymbol n = Reduce(container.begin() + j + 1, container.begin() + k + 1);

			if (!n) {
				Debug::LogError(std::string("failed to reduce `") + Utility::Concat(container.begin() + j + 1, container.begin() + k + 1) + "` at " + position.ToString());
				return false;
			}

			Debug::Log(std::to_string(++reduceCount) + "\tReduce `" + Utility::Concat(container.begin() + j + 1, container.begin() + k + 1) + "` to `" + n.ToString() + "`");

			container.erase(container.begin() + j + 1, container.begin() + k + 1);

			k = j + 1;
			container.push_back(n);
		}

		if (ComparePrecedence(container[j], a, OperatorPrecedenceLess) || ComparePrecedence(container[j], a, OperatorPrecedenceEqual)) {
			++k;
			container.push_back(a);
		}
		else if (!OnUnexpectedToken(a, container, position)) {
			return false;
		}

	} while (a != GrammarSymbol::zero);

	Debug::Log("Accept");
	return true;
}

std::string OperatorPrecedenceParser::ToString() const {
	std::ostringstream oss;
	oss << Parser::ToString();

	oss << "\n\n";

	oss << Utility::Heading(" FirstVt ") << "\n";
	oss << firstVtContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" LastVt ") << "\n";
	oss << lastVtContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" OperatorPrecedenceTable ") << "\n";
	oss << operatorPrecedenceTable_->ToString(terminalSymbols_);

	return oss.str();
}

bool OperatorPrecedenceParser::ParseGrammars() {
	Assert(CheckOperatorGrammar(), "invalid operator grammar");

	CreateFirstVt();
	CreateLastVt();
	CreateParsingTable();

	return true;
}

void OperatorPrecedenceParser::Clear() {
	firstVtContainer_.clear();
	lastVtContainer_.clear();
}

template <class FindTerminalSymbol, class GetEnds>
void OperatorPrecedenceParser::CreateVt(GrammarSymbolSetTable& target, FindTerminalSymbol findTerminalSymbol, GetEnds getEnds) {
	SymbolPairStack s;

	for (GrammarContainer::const_iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		for (CondinateContainer::const_iterator ite = conds.begin();
			ite != conds.end(); ++ite) {
			Condinate* c = *ite;
			GrammarSymbol symbol;
			if (!findTerminalSymbol(symbol, c)) {
				continue;
			}

			if (target[g->GetLhs()].insert(symbol).second) {
				s.push(std::make_pair(g->GetLhs(), symbol));
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
				if (getEnds(c) != item.first) {
					continue;
				}

				if (target[g->GetLhs()].insert(item.second).second) {
					s.push(std::make_pair(g->GetLhs(), item.second));
				}
			}
		}
	}
}

void OperatorPrecedenceParser::CreateFirstVt() {
	CreateVt(firstVtContainer_, CreateVtHelper::FindFirstTerminalSymbol, CreateVtHelper::Front);
}

void OperatorPrecedenceParser::CreateLastVt() {
	CreateVt(lastVtContainer_, CreateVtHelper::FindLastTerminalSymbol, CreateVtHelper::Back);
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

	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols_.begin();
		ite != terminalSymbols_.end(); ++ite) {
		if (ite->second == GrammarSymbol::zero) {
			continue;
		}

		operatorPrecedenceTable_->insert(GrammarSymbol::zero, ite->second, OperatorPrecedenceLess);
		operatorPrecedenceTable_->insert(ite->second, GrammarSymbol::zero, OperatorPrecedenceGreater);
	}
}

void OperatorPrecedenceParser::CreateParsingFunction() {
	/*
	disjoint_sets<GrammarSymbol> sets;
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols_.begin();
		ite != terminalSymbols_.end(); ++ite) {
		sets.make_set(ite->second);
	}

	for (OperatorPrecedenceTable::const_iterator ite = operatorPrecedenceTable_->begin();
		ite != operatorPrecedenceTable_->end(); ++ite) {
		if (ite->second == OperatorPrecedenceEqual) {
			sets.union_set(ite->first.first, ite->first.second);
		}
	}*/
}

void OperatorPrecedenceParser::InsertOperatorPrecedence(const GrammarSymbol& k1, const GrammarSymbol& k2, OperatorPrecedence precedence) {
	OperatorPrecedenceTable::const_iterator pos = operatorPrecedenceTable_->find(k1, k2);
	if (pos == operatorPrecedenceTable_->end()) {
		operatorPrecedenceTable_->insert(k1, k2, precedence);
	}
	else if (precedence != pos->second) {
		Debug::LogWarning(std::string("invalid operator precedence grammar, precedence (")
			+ k1.ToString() + ", " + k2.ToString() + ") : (" + strOperatorPrecedence[pos->second] + ", " + strOperatorPrecedence[precedence] + ")");
	}
}

void OperatorPrecedenceParser::BuildParsingTable(Condinate* c) {
	for (int i = 0; i < (int)c->size() - 1; ++i) {
		GrammarSymbolType tcurrent = c->at(i).SymbolType(), tnext = c->at(i + 1).SymbolType();

		if (tcurrent == GrammarSymbolTerminal && tnext == GrammarSymbolTerminal) {
			InsertOperatorPrecedence(c->at(i), c->at(i + 1), OperatorPrecedenceEqual);
		}

		if (i < (int)c->size() - 2
			&& tcurrent == GrammarSymbolTerminal && tnext == GrammarSymbolNonterminal && c->at(i + 2).SymbolType() == GrammarSymbolTerminal) {
			InsertOperatorPrecedence(c->at(i), c->at(i + 2), OperatorPrecedenceEqual);
		}

		if (tcurrent == GrammarSymbolTerminal && tnext == GrammarSymbolNonterminal) {
			GrammarSymbolSetTable::const_iterator pos = firstVtContainer_.find(c->at(i + 1));
			if (pos != firstVtContainer_.end()) {
				const GrammarSymbolSet& cont = pos->second;
				for (GrammarSymbolSet::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
					InsertOperatorPrecedence(c->at(i), *ite, OperatorPrecedenceLess);
				}
			}
		}

		if (tcurrent == GrammarSymbolNonterminal && tnext == GrammarSymbolTerminal) {
			GrammarSymbolSetTable::const_iterator pos = lastVtContainer_.find(c->at(i));
			if (pos != lastVtContainer_.end()) {
				const GrammarSymbolSet& cont = pos->second;
				for (GrammarSymbolSet::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
					InsertOperatorPrecedence(*ite, c->at(i + 1), OperatorPrecedenceGreater);
				}
			}
		}
	}
}

bool OperatorPrecedenceParser::CheckOperatorGrammar() const {
	struct GrammarSymbolTypeComparer : std::binary_function<GrammarSymbol, GrammarSymbol, bool> {
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

			if (c->size() == 1 && c->front() == GrammarSymbol::epsilon) {
				Debug::LogError("Production is epsilon.");
				return false;
			}

			if (std::adjacent_find(c->begin(), c->end(), GrammarSymbolTypeComparer()) != c->end()) {
				Debug::LogError("Adjacent non-terminal grammar symbol.");
				return false;
			}
			/*
			if (c->size() == 1 && c->front().SymbolType() == GrammarSymbolNonterminal) {
				Debug::LogWarning("production " + g->GetLhs().ToString() + " : " + c->ToString() + " will be omitted.");
			}*/
		}
	}

	return true;
}

bool OperatorPrecedenceParser::OnUnexpectedToken(GrammarSymbol &a, const SymbolStack &container, const TokenPosition &position) {
	if (a == GrammarSymbol::newline) {
		// 忽略无法匹配的换行符.
		return true;
	}

	if (a == GrammarSymbol::zero) {
		if (container.size() != 2 || container.front() != GrammarSymbol::zero || container.back().SymbolType() != GrammarSymbolNonterminal) {
			Debug::LogError("unexpected end of file.");
			return false;
		}

		return true;
	}

	Debug::LogError("unexpected token " + a.ToString() + " at " + position.ToString());
	return false;
}
