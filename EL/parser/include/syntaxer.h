#pragma once

#include "table.h"
#include "grammar.h"
#include "lr_table.h"
#include "grammar_symbol.h"

class SyntaxNode;
class SyntaxTree;
class FileScanner;

struct Environment;
struct TokenPosition;
struct SyntaxerStack;

struct SyntaxerSetupParameter {
	Environment* env;
	LRTable lrTable;
};

class Syntaxer {
public:
	Syntaxer();
	~Syntaxer();

public:
	bool Load(std::ifstream& file);
	bool Save(std::ofstream& file);

public:
	void Setup(const SyntaxerSetupParameter& p);
	bool ParseSyntax(SyntaxTree* tree, FileScanner* fileScanner);

public:
	std::string ToString() const;

private:
	int Reduce(int cpos);
	void Shift(int state, void* addr, const GrammarSymbol& symbol);
	bool Error(const GrammarSymbol& symbol, const TokenPosition& position);

	bool CreateSyntaxTree(SyntaxNode*& root, FileScanner* fileScanner);

	GrammarSymbol FindSymbol(const ScannerToken& token, void*& addr);
	GrammarSymbol ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner);

private:
	SyntaxerStack* stack_;
	SyntaxerSetupParameter p_;

private:
	SymTable* symTable_;
	LiteralTable* literalTable_;
	ConstantTable* constantTable_;
};
