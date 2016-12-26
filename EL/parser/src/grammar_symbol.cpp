#include <sstream>

#include "debug.h"
#include "token.h"
#include "grammar_symbol.h"

GrammarSymbol GrammarSymbol::null = nullptr;
GrammarSymbol GrammarSymbol::zero = new TerminalSymbol("zero");
GrammarSymbol GrammarSymbol::number = new TerminalSymbol("number");
GrammarSymbol GrammarSymbol::string = new TerminalSymbol("string");
GrammarSymbol GrammarSymbol::epsilon = new TerminalSymbol("epsilon");
GrammarSymbol GrammarSymbol::identifier = new TerminalSymbol("identifier");
GrammarSymbol GrammarSymbol::program = new NonterminalSymbol("Program");

std::string GrammarSymbolContainer::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = " ";
		oss << ite->second.ToString();
	}

	return oss.str();
}

std::string GrammarSymbolSetTable::ToString() const {
	std::ostringstream oss;

	const char* newline = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		if (ite->first.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		oss << newline;
		newline = "\n";

		oss.width(22);
		oss.setf(std::ios::left);

		oss << ite->first.ToString();
		oss << "{";

		const char* seperator = "";
		for (GrammarSymbolSet::const_iterator ite2 = ite->second.begin(); ite2 != ite->second.end(); ++ite2) {
			oss << seperator;
			seperator = " ";
			oss << ite2->ToString();
		}

		oss << "}";
	}

	return oss.str();
}

void FirstSetTable::GetFirstSet(GrammarSymbolSet& answer, SymbolVector::iterator first, SymbolVector::iterator last) {
	for (; first != last; ++first) {
		Assert(find(*first) != end(), "logic error");
		GrammarSymbolSet& firstSet = at(*first);
		bool hasEpsilon = false;
		for (GrammarSymbolSet::iterator ite = firstSet.begin(); ite != firstSet.end(); ++ite) {
			if (*ite != GrammarSymbol::epsilon) {
				answer.insert(*ite);
			}
			else {
				hasEpsilon = true;
			}
		}

		if (!hasEpsilon) {
			break;
		}
	}

	if (first == last) {
		answer.insert(GrammarSymbol::epsilon);
	}
}

GrammarSymbol SymbolFactory::Create(const std::string& text) {
	if (Utility::IsTerminal(text)) {
		return new TerminalSymbol(text);
	}

	return new NonterminalSymbol(text);
}
