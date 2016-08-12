#pragma once
#include <list>
#include <vector>
#include "types.h"

class _GrammarSymbol : public RefCountable {
public:
	friend class GrammarSymbol;

	const std::string& ToString() {
		return text_;
	}

	virtual bool Match(const char* text) const = 0;

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

	virtual bool Match(const char* text) const {
		return true;
	}
};

class NonterminalSymbol : public _GrammarSymbol {
public:
	NonterminalSymbol(const char* text) 
		: _GrammarSymbol(text) { 
	}

	virtual bool Match(const char* text) const {
		return true;
	}
};

class Epsilon : public TerminalSymbol {
public:
	Epsilon()
		: TerminalSymbol("epsilon") {
	}
};

class Alphabet : public NonterminalSymbol {
public:
	Alphabet()
		: NonterminalSymbol("$alphabet") {
	}
};

class Digits : public NonterminalSymbol {
public:
	Digits()
		: NonterminalSymbol("$digits") {
	}
};

class GrammarSymbolContainer {
public:
	GrammarSymbolContainer();
	GrammarSymbol AddSymbol(const char* text, bool terminal);

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
	bool operator == (const GrammarSymbol& other) const;
	bool operator != (const GrammarSymbol& other) const;

public:
	std::string ToString() const;

public:
	static GrammarSymbol epsilon;
	static GrammarSymbol alphabet;
	static GrammarSymbol digits;

private:
	void* operator new(size_t);

private:
	_GrammarSymbol* symbol_;
};

typedef std::vector<GrammarSymbol> Condinate;
typedef std::vector<Condinate*> CondinateContainer;

class Grammar {
public:
	Grammar();
	Grammar(const GrammarSymbol& left);
	~Grammar();

public:
	void SetLeft(const GrammarSymbol& symbol);
	const GrammarSymbol& GetLeft() const;
	const CondinateContainer& GetCondinates() const;

	Condinate* AddCondinate();
	
	void SortCondinates();

	std::string ToString() const;

private:
	GrammarSymbol left_;
	CondinateContainer condinates_;
};

typedef std::list<Grammar*> GrammarContainer;
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
	bool ParseProductions(LineScanner* lineScanner, GrammarSymbolContainer* symbols);
	bool ParseGrammar();

	void RemoveLeftRecursion();
	bool HandleLeftRecursion(Grammar* g, GrammarContainer* newGrammars);

	void CreateFirstSets();
	void CreateFollowSets();
private:
	GrammarContainer grammars_;
};
