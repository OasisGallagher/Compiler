#pragma once
#include <set>
#include <map>
#include "utilities.h"
#include "reference_countable.h"

enum GrammarSymbolType {
	GrammarSymbolTerminal,
	GrammarSymbolNonterminal,
};

class GrammarSymbolBase : public RefCountable {
public:
	friend class GrammarSymbol;
	friend class SymbolFactory;

	const std::string& ToString() {
		return text_;
	}

	virtual GrammarSymbolType SymbolType() const = 0;
	virtual bool Match(const std::string& text) const = 0;

protected:
	GrammarSymbolBase(const std::string& text)
		: text_(text) {
	}

	~GrammarSymbolBase() {
	}

protected:
	static int ninstance;
	std::string text_;
};

class TerminalSymbol : public GrammarSymbolBase {
public:
	TerminalSymbol(const std::string& text)
		: GrammarSymbolBase(text) {
	}

	virtual GrammarSymbolType SymbolType() const {
		return GrammarSymbolTerminal;
	}

	virtual bool Match(const std::string& text) const {
		return text_ == text;
	}
};

class NonterminalSymbol : public GrammarSymbolBase {
public:
	NonterminalSymbol(const std::string& text)
		: GrammarSymbolBase(text) {
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

	virtual bool Match(const std::string& text) const {
		Assert(false, "unable to compare zero with text");
		return false;
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

class String : public TerminalSymbol {
public:
	String()
		: TerminalSymbol("string") {
	}

	virtual bool Match(const std::string& text) const {
		return true;
	}
};

class SymbolFactory {
public:
	static GrammarSymbol Create(const std::string& text);
};

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
	bool Match(const std::string& text) const;
	std::string ToString() const;

public:
	static GrammarSymbol null;
	static GrammarSymbol zero;
	static GrammarSymbol number;
	static GrammarSymbol string;
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
