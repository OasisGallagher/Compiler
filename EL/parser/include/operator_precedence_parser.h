#pragma once
#include "parser.h"

class OperatorPrecedenceTable;

class OperatorPrecedenceParser : public Parser {
public:
	OperatorPrecedenceParser();
	~OperatorPrecedenceParser();

public:
	virtual bool ParseFile(SyntaxTree* tree, FileScanner* fileScanner);
	virtual std::string ToString() const;

protected:
	virtual bool ParseGrammars();
	virtual void Clear();

private:
	void CreateFirstVt();
	void CreateLastVt();
	void CreateParsingTable();
	void BuildParsingTable(Condinate* c);

	bool IsOperatorGrammar() const;

private:
	GrammarSymbolSetTable firstVtContainer_;
	GrammarSymbolSetTable lastVtContainer_;
	OperatorPrecedenceTable* operatorPrecedenceTable_;
};
