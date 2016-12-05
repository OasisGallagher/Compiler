#pragma once
#include "grammar.h"

class SyntaxTree;
class FileScanner;
class TextScanner;

class SymTable;
class LiteralTable;
class ConstantTable;

struct ScannerToken;

class Parser {
public:
	Parser();
	virtual ~Parser();

public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner) = 0;
	virtual std::string ToString() const;

public:
	bool SetGrammars(const char* productions);

protected:
	virtual bool ParseGrammars() = 0;
	virtual void Clear();

protected:
	GrammarSymbol FindSymbol(const ScannerToken& token, void*& addr);
	Grammar* FindGrammar(const GrammarSymbol& lhs, int* index = nullptr);
	GrammarSymbol CreateSymbol(const std::string& text);
	bool MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src);

	void CreateFirstSets();
	void CreateFollowSets();

protected:
	GrammarContainer grammars_;
	GrammarSymbolContainer terminalSymbols_;
	GrammarSymbolContainer nonterminalSymbols_;

	// VnµÄfirst/follow¼¯ºÏ.
	FirstSetTable firstSetContainer_;
	GrammarSymbolSetTable followSetContainer_;

private:
	void InitializeTerminalSymbolContainer();
	bool ParseProductions(TextScanner* textScanner, SymbolVector& symbols);
	void DestroyGammars();

	bool CreateFirstSetsOnePass();
	bool CreateFollowSetsOnePass();

private:
	SymTable* symTable_;
	LiteralTable* literalTable_;
	ConstantTable* constantTable_;
};
