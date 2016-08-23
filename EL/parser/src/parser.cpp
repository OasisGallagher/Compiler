#include <sstream>

#include "parser.h"

BuildinSymbolContainer::BuildinSymbolContainer() {
	insert(std::make_pair(GrammarSymbol::string.ToString(), GrammarSymbol::string));
	insert(std::make_pair(GrammarSymbol::number.ToString(), GrammarSymbol::number));
	insert(std::make_pair(GrammarSymbol::epsilon.ToString(), GrammarSymbol::epsilon));
	insert(std::make_pair(GrammarSymbol::identifier.ToString(), GrammarSymbol::identifier));
}

std::string BuildinSymbolContainer::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		oss << seperator;
		seperator = " ";
		oss << ite->second.ToString();
	}

	return oss.str();
}
