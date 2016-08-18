#pragma once
#include <set>
#include <map>
#include "utilities.h"
#include "reference_countable.h"

enum GrammarSymbolType {
	GrammarSymbolTerminal,
	GrammarSymbolNonterminal,
};

class _GrammarSymbol : public RefCountable {
public:
	friend class GrammarSymbol;

	const std::string& ToString() {
		return text_;
	}

	virtual GrammarSymbolType SymbolType() const = 0;
	virtual bool Match(const std::string& text) const = 0;

protected:
	_GrammarSymbol(const std::string& text)
		: text_(text) {
	}

	~_GrammarSymbol() {
	}

protected:
	std::string text_;
};

class TerminalSymbol : public _GrammarSymbol {
public:
	TerminalSymbol(const std::string& text)
		: _GrammarSymbol(text) {
	}

	virtual GrammarSymbolType SymbolType() const {
		return GrammarSymbolTerminal;
	}

	virtual bool Match(const std::string& text) const {
		return text_ == text;
	}
};

class NonterminalSymbol : public _GrammarSymbol {
public:
	NonterminalSymbol(const std::string& text)
		: _GrammarSymbol(text) {
	}

	virtual GrammarSymbolType SymbolType() const {
		return GrammarSymbolNonterminal;
	}

	virtual bool Match(const std::string& text) const {
		Assert(false, "match nonterminal symbol");
		return false;
	}
};

class Zero : public TerminalSymbol {
public:
	Zero()
		: TerminalSymbol("zero") {
	}
};

class Epsilon : public TerminalSymbol {
public:
	Epsilon()
		: TerminalSymbol("epsilon") {
	}

	virtual bool Match(const std::string& text) const {
		return true;
	}
};

class Identifier : public TerminalSymbol {
public:
	Identifier()
		: TerminalSymbol("identifier") {
	}

	virtual bool Match(const std::string& text) const {
		return true;
	}
};

class Number : public TerminalSymbol {
public:
	Number()
		: TerminalSymbol("number") {
	}

	virtual bool Match(const std::string& text) const {
		return true;
	}
};

class String : public TerminalSymbol{
public:
	String()
		: TerminalSymbol("string") {
	}

	virtual bool Match(const std::string& text) const {
		return true;
	}
};

class GrammarSymbolContainer {
private:
	typedef std::map<std::string, GrammarSymbol> Container;

public: 
	typedef Container::const_iterator const_iterator;

public:
	GrammarSymbolContainer();

public:
	GrammarSymbol AddSymbol(const std::string& text, bool terminal);

	
	GrammarSymbol& operator[](const std::string& text);
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator find(const std::string& text) const;
private:
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
	operator bool() const;
	bool operator == (const GrammarSymbol& other) const;
	bool operator != (const GrammarSymbol& other) const;
	bool operator < (const GrammarSymbol& other) const;
	bool operator > (const GrammarSymbol& other) const;

public:
	GrammarSymbolType SymbolType() const;
	bool Match(const std::string& text) const;
	std::string ToString() const;

public:
	static GrammarSymbol zero;
	static GrammarSymbol number;
	static GrammarSymbol string;
	static GrammarSymbol epsilon;
	static GrammarSymbol identifier;

private:
	void* operator new(size_t);

private:
	_GrammarSymbol* symbol_;
};

typedef std::set<GrammarSymbol> GrammarSymbolSet;

class GrammarSymbolSetTable {
public:
	GrammarSymbolSetTable();

public:
	GrammarSymbolSet& operator[] (const GrammarSymbol& key);
	std::string ToString() const;

private:
	typedef std::map<GrammarSymbol, GrammarSymbolSet> Container;
	Container cont_;
};
