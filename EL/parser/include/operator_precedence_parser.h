#pragma once
#include "parser.h"

class OperatorPrecedenceTable;

class OperatorPrecedenceParser : public Parser {
public:
	OperatorPrecedenceParser();
	~OperatorPrecedenceParser();

public:
	enum OperatorPrecedence {
		OperatorPrecedenceEmpty,
		OperatorPrecedenceLess,
		OperatorPrecedenceEqual,
		OperatorPrecedenceGreater,
	};

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
	void CreateParsingFunction();

	void AddZeroSymbol();
	void BuildParsingTable(Condinate* c);

	template <class Iterator>
	GrammarSymbol Reduce(Iterator first, Iterator last);

	template <class Iterator>
	bool MatchProduction(const Condinate* c, Iterator first, Iterator last) const;

	bool IsOperatorGrammar() const;
	bool ComparePrecedence(const GrammarSymbol& lhs, const GrammarSymbol& rhs, OperatorPrecedence precedence) const;
	
private:
	GrammarSymbolSetTable firstVtContainer_;
	GrammarSymbolSetTable lastVtContainer_;
	OperatorPrecedenceTable* operatorPrecedenceTable_;
};
