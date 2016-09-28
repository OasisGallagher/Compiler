#pragma once
#include <list>
#include <vector>
#include "grammar_symbol.h"

class Action;

struct Condinate {
	std::string ToString() const;

	SymbolVector symbols;
	Action* action;
};

class CondinateContainer :public std::vector<Condinate*> {
public:
	std::string ToString() const;
};

class Grammar {
public:
	Grammar();
	Grammar(const GrammarSymbol& lhs);
	~Grammar();

public:
	void SetLhs(const GrammarSymbol& symbol);
	void AddCondinate(const Condinate& cond);
	void SortCondinates();

	const GrammarSymbol& GetLhs() const;
	const CondinateContainer& GetCondinates() const;

	std::string ToString() const;

private:
	GrammarSymbol lhs_;
	CondinateContainer condinates_;
};

typedef std::list<Grammar*> GrammarContainer;
