#pragma once
#include <vector>
#include "types.h"

class _GrammarSymbol : public RefCountable {
public:
	friend class GrammarSymbol;

	const std::string& ToString() {
		return text_;
	}

protected:
	_GrammarSymbol(const char* text)
		: text_(text) {
	}

	~_GrammarSymbol() {
	}

protected:
	std::string text_;
};

class TerminalSymbol : public _GrammarSymbol {
public:
	TerminalSymbol(const char* text)
		: _GrammarSymbol(text) {
	}
};

class NonterminalSymbol : public _GrammarSymbol {
public:
	NonterminalSymbol(const char* text) 
		: _GrammarSymbol(text) { 
	}
};

class GrammarSymbolContainer {
public:
	GrammarSymbol AddSymbol(const char* text, bool terminal);
	void Clear();

private:
	typedef std::map<std::string, GrammarSymbol> Container;
	Container cont_;
};

class GrammarSymbol {
public:
	GrammarSymbol();
	GrammarSymbol(_GrammarSymbol* symbol);

	GrammarSymbol(const GrammarSymbol& other);

	GrammarSymbol& operator = (const GrammarSymbol& other);

	~GrammarSymbol();

public:
	std::string ToString();

private:
	void* operator new(size_t);

private:
	_GrammarSymbol* symbol_;
};

typedef std::vector<GrammarSymbol> Condinate;
typedef std::vector<Condinate*> CondinateContainer;

class Grammar {
public:
	Grammar(const GrammarSymbol& left);
	~Grammar();

public:
	Condinate* AddCondinate();

	std::string ToString();

private:
	GrammarSymbol left_;
	CondinateContainer condinates_;
};

typedef std::vector<Grammar*> GrammarContainer;
class LineScanner;

class GrammarParser {
public:
	GrammarParser();
	~GrammarParser();

public:
	bool Parse(const char* productions[], int count);

	std::string ToString();

private:
	bool IsTerminal(const char* token);
	bool ParseProduction(LineScanner* lineScanner, GrammarSymbolContainer* symbols);
	bool ParseGrammar();

private:
	GrammarContainer grammars_;
};
