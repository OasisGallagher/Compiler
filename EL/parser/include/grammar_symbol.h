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

class GrammarSymbolBase {
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
	TerminalSymbol(const std::string& text);

public:
	virtual GrammarSymbolType SymbolType() const;
	virtual bool Match(const ScannerToken& token) const;
};

class NonterminalSymbol : public GrammarSymbolBase {
public:
	NonterminalSymbol(const std::string& text);

public:
	virtual GrammarSymbolType SymbolType() const;
	virtual bool Match(const ScannerToken& token) const;
};

class Zero : public TerminalSymbol {
public:
	Zero();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Epsilon : public TerminalSymbol {
public:
	Epsilon();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Identifier : public TerminalSymbol {
public:
	Identifier();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Negative : public TerminalSymbol {
public:
	Negative();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Positive : public TerminalSymbol {
public:
	Positive();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Number : public TerminalSymbol {
public:
	Number();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class String : public TerminalSymbol {
public:
	String();

public:
	virtual bool Match(const ScannerToken& token) const;
};

class Newline : public TerminalSymbol {
public:
	Newline();

public:
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
	IMPLEMENT_REFERENCE_COUNTABLE(GrammarSymbol, GrammarSymbolBase);

public:
	GrammarSymbol();

public:
	operator bool() const;

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
	static GrammarSymbol negative;
	static GrammarSymbol positive;
	static GrammarSymbol identifier;

private:
#pragma push_macro("new")
#undef new
	// GrammarSymbol不可以通过new分配.
	void* operator new(size_t);
#pragma pop_macro("new")
};

typedef std::set<GrammarSymbol> GrammarSymbolSet;

class GrammarSymbolSetTable : public std::map<GrammarSymbol, GrammarSymbolSet> {
public:
	std::string ToString() const;
};

class FirstSetTable : public GrammarSymbolSetTable {
public:
	void GetFirstSet(GrammarSymbolSet& answer, SymbolVector::iterator first, SymbolVector::iterator last);
};
