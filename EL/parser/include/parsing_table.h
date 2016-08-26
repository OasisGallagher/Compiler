#pragma once

#include "grammar.h"
#include "matrix.h"

class ParsingTable : public matrix<GrammarSymbol, GrammarSymbol, std::pair<GrammarSymbol, Condinate*> > {
public:
	std::string ToString() const;
};
