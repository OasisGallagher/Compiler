#pragma once
#include <set>
#include <list>
#include <vector>
#include "grammar.h"

class LineScanner;
class ParsingTable;
class SyntaxTree;

class Language {
public:
	Language();
	~Language();

public:
	bool SetGrammars(const char* productions[], int count);
	bool Parse(SyntaxTree** tree, FileScanner* fileScanner);

	std::string ToString();

private:

	bool IsTerminal(const char* token);
	bool ParseProductions(LineScanner* lineScanner, GrammarSymbolContainer* symbols);
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

private:
	ParsingTable* parsingTable_;
	GrammarContainer grammars_;
	GrammarSymbolSetTable firstSetContainer_, followSetContainer_;
};
