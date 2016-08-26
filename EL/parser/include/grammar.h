#pragma once
#include <list>
#include <vector>
#include "grammar_symbol.h"

class Condinate : public std::vector<GrammarSymbol> {
public:
	std::string ToString() const;
};

class CondinateContainer :public std::vector<Condinate*> {
public:
	std::string ToString() const;
};

class Grammar {
public:
	Grammar();
	Grammar(const GrammarSymbol& left);
	~Grammar();

public:
	void SetLeft(const GrammarSymbol& symbol);
	void AddCondinate(const Condinate& cond);
	void SortCondinates();

	const GrammarSymbol& GetLeft() const;
	const CondinateContainer& GetCondinates() const;

	std::string ToString() const;

private:
	GrammarSymbol left_;
	CondinateContainer condinates_;
};

typedef std::list<Grammar*> GrammarContainer;
