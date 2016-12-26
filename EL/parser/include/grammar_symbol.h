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

	const std::string& ToString() { return text_; }

public:
	virtual GrammarSymbolType SymbolType() const = 0;

protected:
	GrammarSymbolBase(const std::string& text) : text_(text) { }
	~GrammarSymbolBase() { }

protected:
	std::string text_;
};

class TerminalSymbol : public GrammarSymbolBase {
public:
	TerminalSymbol(const std::string& text) : GrammarSymbolBase(text) { }

public:
	virtual GrammarSymbolType SymbolType() const { return GrammarSymbolTerminal; }
};

class NonterminalSymbol : public GrammarSymbolBase {
public:
	NonterminalSymbol(const std::string& text) : GrammarSymbolBase(text) { }

public:
	virtual GrammarSymbolType SymbolType() const { return GrammarSymbolNonterminal; }
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
	GrammarSymbol() : ptr_(nullptr) { }

public:
	operator bool() const { return ptr_ != nullptr; }

public:
	bool operator == (const GrammarSymbol& other) const { return ptr_ == other.ptr_; }
	bool operator != (const GrammarSymbol& other) const { return ptr_ != other.ptr_; }
	bool operator < (const GrammarSymbol& other) const { return ptr_ < other.ptr_; }
	bool operator >(const GrammarSymbol& other) const { return ptr_ > other.ptr_; }

public:
	GrammarSymbolType SymbolType() const { return ptr_->SymbolType(); }
	std::string ToString() const { return (ptr_ != nullptr) ? ptr_->ToString() : "null"; }

public:
	static GrammarSymbol null;
	static GrammarSymbol zero;
	static GrammarSymbol number;
	static GrammarSymbol string;
	static GrammarSymbol epsilon;
	static GrammarSymbol program;
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
