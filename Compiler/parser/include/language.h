#pragma once
#include <string>
#include "grammar_symbol.h"

class Syntaxer;
class SyntaxTree;
class TextScanner;

struct Environment;

class Language {
public:
	Language();
	~Language();

public:
	void Setup(const char* productions, const char* fileName);

public:
	bool Parse(SyntaxTree* tree, const std::string& file);
	std::string ToString() const;

private:
	void SaveSyntaxer(const char* fileName);
	void LoadSyntaxer(const char* fileName);
	void BuildSyntaxer(const char* productions);

	bool SetupEnvironment(const char* productions);
	GrammarSymbol CreateSymbol(const std::string& text);
	bool ParseProductions(const char* productions);
	bool ParseProduction(TextScanner* textScanner, SymbolVector& symbols);

private:
	Environment* env_;
	Syntaxer* syntaxer_;
};
