#pragma once
#include "grammar.h"

class Syntaxer;
class SyntaxTree;
class FileScanner;
class TextScanner;

struct ScannerToken;

struct Environment {
	GrammarContainer grammars;
	GrammarSymbolContainer terminalSymbols;
	GrammarSymbolContainer nonterminalSymbols;

	bool Load(std::ifstream& file);
	bool Save(std::ofstream& file);

	~Environment();
};

class Parser {
public:
	Parser();
	virtual ~Parser();

public:
	virtual std::string ToString() const;

public:
	bool Setup(Syntaxer& syntaxer, Environment* env);

protected:
	virtual bool ParseGrammars(Syntaxer& syntaxer, Environment* env) = 0;
	virtual void Clear();

protected:
	Grammar* FindGrammar(const GrammarSymbol& lhs, int* index = nullptr);
	GrammarSymbol CreateSymbol(const std::string& text);
	bool MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src);

	void CreateFirstSets();
	void CreateFollowSets();

protected:
	Environment* env_;

	// VnµÄfirst/follow¼¯ºÏ.
	FirstSetTable firstSetContainer_;
	GrammarSymbolSetTable followSetContainer_;

private:
	bool CreateFirstSetsOnePass();
	bool CreateFollowSetsOnePass();
};
