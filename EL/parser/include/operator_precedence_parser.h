#pragma once
#include "parser.h"

class OperatorPrecedenceTable;

class OperatorPrecedenceParser : public Parser {
public:
	OperatorPrecedenceParser();
	~OperatorPrecedenceParser();

public:
	enum OperatorPrecedence {
		OperatorPrecedenceUndefined,
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
	GrammarSymbol Reduce(Iterator first, Iterator last, Condinate** condinate);

	template <class Iterator>
	bool MatchProduction(const Condinate* c, Iterator first, Iterator last) const;

	template <class FindTerminalSymbol, class GetEnds>
	void CreateVt(GrammarSymbolSetTable& target, FindTerminalSymbol findTerminalSymbol, GetEnds getEnds);

	bool OnUnexpectedToken(GrammarSymbol &a, const SymbolVector& container, const TokenPosition &position);
	bool CheckOperatorGrammar() const;
	void InsertOperatorPrecedence(const GrammarSymbol& k1, const GrammarSymbol& k2, OperatorPrecedence precedence);

	OperatorPrecedence GetPrecedence(const GrammarSymbol& lhs, const GrammarSymbol& rhs) const;

private:
	GrammarSymbolSetTable firstVtContainer_;
	GrammarSymbolSetTable lastVtContainer_;
	OperatorPrecedenceTable* operatorPrecedenceTable_;
};
