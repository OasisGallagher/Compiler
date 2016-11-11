#pragma once
#include <list>
#include <vector>
#include "grammar_symbol.h"

class Action;

struct Condinate {
	Condinate(const SymbolVector& container, const std::string& actionText);
	~Condinate();

	std::string ToString() const;

	SymbolVector symbols;
	Action* action;

private:
	Condinate(const Condinate&);
	Condinate& operator = (const Condinate&);
};

class CondinateContainer : public std::vector<Condinate*> {
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
	void AddCondinate(const std::string& action, const SymbolVector& symbols);
	void SortCondinates();

	const GrammarSymbol& GetLhs() const;
	const CondinateContainer& GetCondinates() const;

	std::string ToString() const;

private:
	GrammarSymbol lhs_;
	CondinateContainer condinates_;
};

class GrammarContainer : public std::list<Grammar*> {
public:
	Grammar* FindGrammar(const GrammarSymbol& lhs, int* index);
};
