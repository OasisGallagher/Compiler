#pragma once
#include <vector>
#include "types.h"

class GrammarSymbol {
public:
	const std::string& ToString() {
		return text_;
	}

protected:
	GrammarSymbol(const char* text)
		: text_(text) {
	}

protected:
	std::string text_;
};

class TerminalSymbol : public GrammarSymbol {
public:
	TerminalSymbol(const char* text)
		: GrammarSymbol(text) {
	}
};

class NonterminalSymbol : public GrammarSymbol {
public:
	NonterminalSymbol(const char* text) 
		: GrammarSymbol(text) { 
	}
};

class TerminalSymbolContainer : public Table<TerminalSymbol>{
};

class NonterminalSymbolContainer : public Table<NonterminalSymbol>{
};

typedef std::vector<GrammarSymbol*> Condinate;
typedef std::vector<Condinate*> CondinateContainer;

class Grammar {
public:
	Grammar();
	~Grammar();

public:
	void SetLeft(GrammarSymbol* left);
	Condinate* AddCondinate();

	std::string ToString();

private:
	GrammarSymbol* left_;
	CondinateContainer condinates_;
};

typedef std::vector<Grammar*> GrammarContainer;
class LineScanner;

class GrammarParser {
public:
	GrammarParser();
	~GrammarParser();

public:
	bool AddProduction(const char* production);
	std::string ToString();

private:
	bool IsTerminal(const char* token);
	bool ParseProduction();

private:
	GrammarContainer grammars_;
	LineScanner* lineScanner_;
	TerminalSymbolContainer terminalSymbols_;
	NonterminalSymbolContainer nonterminalSymbols_;
};
