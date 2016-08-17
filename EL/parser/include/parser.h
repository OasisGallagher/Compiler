#pragma once
#include <map>
#include <vector>
#include "grammar.h"

class LineScanner;
class ParsingTable;
class SyntaxTree;

class BuildinSymbolContainer : public std::map<std::string, GrammarSymbol> {
public:
	std::string ToString() const;
};

struct LanguageParameter {
	const char** productions;
	int nproductions;
};

class Language {
public:
	Language(LanguageParameter* parameter);
	~Language();

public:
	bool Parse(SyntaxTree** tree, const std::string& file);
	std::string ToString();

private:
	bool SetGrammars(const char** productions, int count);

	bool IsTerminal(const char* token);
	bool IsMnemonic(const char* token);

	bool ParseProductions(LineScanner* lineScanner);
	bool ParseGrammars();

	void RemoveLeftRecursion();
	bool RemoveImmidiateLeftRecursion(Grammar* g, GrammarContainer* newGrammars);

	int LongestCommonPrefix(const Condinate* first, const Condinate* second);
	bool CalculateLongestFactor(Grammar* g, int* range, int* length);
	void LeftFactoring();
	bool LeftFactoringOnGrammar(Grammar* g, GrammarContainer* newGrammars);

	Grammar* FindGrammar(const GrammarSymbol& left);

	bool MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src);

	void CreateFirstSets();
	bool CreateFirstSetsOnePass();
	void GetFirstSet(GrammarSymbolSet* answer, Condinate::iterator first, Condinate::iterator last);

	void CreateFollowSets();
	bool CreateFollowSetsOnePass();

	bool BuildParingTable(Grammar* g);
	bool CreateParsingTable();

	bool ParseFile(SyntaxTree** tree, FileScanner* fileScanner);

private:
	GrammarContainer grammars_;
	ParsingTable* parsingTable_;

	BuildinSymbolContainer buildinSymbols_;
	GrammarSymbolContainer symbolContainer_;

	GrammarSymbolSetTable firstSetContainer_;
	GrammarSymbolSetTable followSetContainer_;
};
