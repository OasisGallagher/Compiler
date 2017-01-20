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
	SyntaxNode* CreateSyntaxTree(FileScanner* fileScanner);

	GrammarSymbol FindSymbol(const ScannerToken& token, void*& addr);
	GrammarSymbol ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner);

private:
	SyntaxerSetupParameter p_;

private:
	SymTable* symTable_;
	LiteralTable* literalTable_;
	ConstantTable* constantTable_;
};
