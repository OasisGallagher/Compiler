#pragma once
#include <set>
#include <map>
#include <vector>
#include "utilities.h"
#include "reference_countable.h"

enum GrammarSymbolType {
	GrammarSymbolTerminal,
	GrammarSymbolNonterminal,
};

struct ScannerToken;

class GrammarSymbolBase : public RefCountable {
public:
	friend class GrammarSymbol;
	friend class SymbolFactory;

	const std::string& ToString();

	virtual GrammarSymbolType SymbolType() const = 0;
	virtual bool Match(const ScannerToken& token) const = 0;

protected:
	GrammarSymbolBase(const std::string& text);

	~GrammarSymbolBase();

protected:
	std::string text_;
};

class TerminalSymbol : public GrammarSymbolBase {
public:
	TerminalSymbol(const std::string& text)
		: GrammarSymbolBase(text) {
	}

	virtual GrammarSymbolType SymbolType() const;

	virtual bool Match(const ScannerToken& token) const;
};

class NonterminalSymbol : public GrammarSymbolBase {
public:
	NonterminalSymbol(const std::string& text)
		: GrammarSymbolBase(text) {
	}

	virtual GrammarSymbolType SymbolType() const;

	virtual bool Match(const ScannerToken& token) const;
};

class Zero : public TerminalSymbol {
public:
	Zero()
		: TerminalSymbol("zero") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class Epsilon : public TerminalSymbol {
public:
	Epsilon()
		: TerminalSymbol("epsilon") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class Identifier : public TerminalSymbol {
public:
	Identifier()
		: TerminalSymbol("identifier") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class Number : public TerminalSymbol {
public:
	Number()
		: TerminalSymbol("number") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class String : public TerminalSymbol {
public:
	String()
		: TerminalSymbol("string") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class Newline : public TerminalSymbol {
public:
	Newline()
		: TerminalSymbol("newline") {
	}

	virtual bool Match(const ScannerToken& token) const;
};

class SymbolFactory {
public:
	static GrammarSymbol Create(const std::string& text);
};

typedef std::vector<GrammarSymbol> SymbolVector;

class GrammarSymbolContainer : public std::map<std::string, GrammarSymbol> {
public:
	std::string ToString() const;
};

class GrammarSymbol {
public:
	GrammarSymbol();
	GrammarSymbol(GrammarSymbolBase* symbol);
	GrammarSymbol(const GrammarSymbol& other);
	~GrammarSymbol();

public:
	operator bool() const;
	GrammarSymbol& operator = (const GrammarSymbol& other);

public:
	bool operator == (const GrammarSymbol& other) const;
	bool operator != (const GrammarSymbol& other) const;
	bool operator < (const GrammarSymbol& other) const;
	bool operator > (const GrammarSymbol& other) const;

public:
	GrammarSymbolType SymbolType() const;
	bool Match(const ScannerToken& token) const;
	std::string ToString() const;

public:
	static GrammarSymbol null;
	static GrammarSymbol zero;
	static GrammarSymbol number;
	static GrammarSymbol string;
	static GrammarSymbol newline;
	static GrammarSymbol epsilon;
	static GrammarSymbol identifier;

private:
#pragma push_macro("new")
#undef new
	// GrammarSymbol不可以通过new分配.
	void* operator new(size_t);
#pragma pop_macro("new")

	static GrammarSymbol synch;

private:
	GrammarSymbolBase* symbol_;
};

typedef std::set<GrammarSymbol> GrammarSymbolSet;

class GrammarSymbolSetTable : public std::map<GrammarSymbol, GrammarSymbolSet> {
public:
	std::string ToString() const;
};
