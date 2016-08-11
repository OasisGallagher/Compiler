#include <cassert>
#include <sstream>
#include "../scanner/scanner.h"
#include "parser.h"
#include "constants.h"

Grammar::Grammar() {
}

Grammar::~Grammar() {
	for (CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		delete *ite;
	}
}

void Grammar::SetLeft(GrammarSymbol* left) {
	left_ = left;
}

Condinate* Grammar::AddCondinate() {
	Condinate* ans = new Condinate();
	condinates_.push_back(ans);
	return ans;
}

std::string Grammar::ToString() {
	std::ostringstream os;
	os << left_->ToString();
	os << " : ";

	char* space = "", *xor = "";
	for (CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		space = "";

		os << xor;
		xor = "|";

		for (Condinate::iterator ite2 = (*ite)->begin(); ite2 != (*ite)->end(); ++ite2) {
			os << space << (*ite2)->ToString();
			space = " ";
			std::string tmp = os.str();
		}
	}

	return os.str();
}

GrammarParser::GrammarParser() {
	lineScanner_ = new LineScanner();
}

GrammarParser::~GrammarParser() {
	delete lineScanner_;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		delete *ite;
	}
}

bool GrammarParser::AddProduction(const char* production) {
	lineScanner_->SetText(production);
	return ParseProduction();
}

bool GrammarParser::ParseProduction() {
	char token[Constants::kMaxTokenCharacters];

	Grammar* grammar = new Grammar();

	ScannerTokenType tokenType = lineScanner_->GetToken(token);
	assert(tokenType != ScannerTokenEndOfFile);

	grammar->SetLeft(terminalSymbols_.Add(token));
	Condinate* cond = grammar->AddCondinate();

	lineScanner_->GetToken(token);
	assert(strcmp(token, ":") == 0);

	for (; (tokenType = lineScanner_->GetToken(token)) != ScannerTokenEndOfFile; ) {
		if (tokenType == ScannerTokenXor) {
			assert(!cond->empty());
			cond = grammar->AddCondinate();
			continue;
		}

		if (IsTerminal(token)) {
			cond->push_back(terminalSymbols_.Add(token));
		}
		else {
			cond->push_back(nonterminalSymbols_.Add(token));
		}
	}

	printf((grammar->ToString() + "\n").c_str());
	grammars_.push_back(grammar);

	return true;
}

bool GrammarParser::IsTerminal(const char* token) {
	return (*token == '_' && strlen(token) > 1);
}
