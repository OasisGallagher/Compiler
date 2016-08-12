#include <cassert>
#include <sstream>
#include "../scanner/scanner.h"
#include "parser.h"
#include "constants.h"

GrammarSymbol GrammarSymbolContainer::AddSymbol(const char* text, bool terminal) {
	Container::iterator ite = cont_.find(text);
	GrammarSymbol ans(nullptr);

	if (ite == cont_.end()) {
		if (terminal) {
			ans = GrammarSymbol(new TerminalSymbol(text));
		}
		else {
			ans = GrammarSymbol(new NonterminalSymbol(text));
		}

		cont_[text] = ans;
	}
	else {
		ans = ite->second;
	}

	return ans;
}

void GrammarSymbolContainer::Clear() {
	cont_.clear();
}

Grammar::Grammar(const GrammarSymbol& left) : left_(left) {
}

Grammar::~Grammar() {
	for (CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		delete *ite;
	}
}

Condinate* Grammar::AddCondinate() {
	Condinate* ans = new Condinate();
	condinates_.push_back(ans);
	return ans;
}

std::string Grammar::ToString() {
	std::ostringstream os;
	os << left_.ToString();
	os << " : ";

	char* space = "", *xor = "";
	for (CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		space = "";

		os << xor;
		xor = "|";

		for (Condinate::iterator ite2 = (*ite)->begin(); ite2 != (*ite)->end(); ++ite2) {
			os << space << ite2->ToString();
			space = " ";
			std::string tmp = os.str();
		}
	}

	return os.str();
}

GrammarParser::GrammarParser() {
}

GrammarParser::~GrammarParser() {
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		delete *ite;
	}
}

bool GrammarParser::Parse(const char* productions[], int count) {
	GrammarSymbolContainer cont;
	LineScanner lineScanner;
	for (int i = 0; i < count; ++i) {
		lineScanner.SetText(productions[i]);
		if (!ParseProduction(&lineScanner, &cont)) {
			return false;
		}
	}

	return ParseGrammar();
}

bool GrammarParser::ParseGrammar() {
	return true;
}

bool GrammarParser::ParseProduction(LineScanner* lineScanner, GrammarSymbolContainer* symbols) {
	char token[Constants::kMaxTokenCharacters];

	ScannerTokenType tokenType = lineScanner->GetToken(token);
	assert(tokenType != ScannerTokenEndOfFile);

	Grammar* grammar = new Grammar(symbols->AddSymbol(token, false));

	Condinate* cond = grammar->AddCondinate();

	lineScanner->GetToken(token);
	Assert(strcmp(token, ":") == 0, "invalid production");

	for (; (tokenType = lineScanner->GetToken(token)) != ScannerTokenEndOfFile; ) {
		if (tokenType == ScannerTokenXor) {
			assert(!cond->empty());
			cond = grammar->AddCondinate();
			continue;
		}

		cond->push_back(symbols->AddSymbol(token, IsTerminal(token)));
	}

	grammars_.push_back(grammar);

	return true;
}

bool GrammarParser::IsTerminal(const char* token) {
	return (*token == '_' && strlen(token) > 1);
}

GrammarSymbol::GrammarSymbol()
	: symbol_(nullptr) {
}

GrammarSymbol::GrammarSymbol(_GrammarSymbol* symbol)
	: symbol_(symbol) {
}

GrammarSymbol::GrammarSymbol(const GrammarSymbol& other) {
	symbol_ = other.symbol_;
	if (symbol_ != nullptr) {
		symbol_->IncRefCount();
	}
}

GrammarSymbol& GrammarSymbol::operator=(const GrammarSymbol& other) {
	if (other.symbol_ != nullptr) {
		other.symbol_->IncRefCount();
	}

	if (symbol_ != nullptr && symbol_->DecRefCount() == 0) {
		delete symbol_;
	}

	symbol_ = other.symbol_;

	return *this;
}

GrammarSymbol::~GrammarSymbol() {
	if (symbol_ != nullptr && symbol_->DecRefCount() == 0) {
		delete symbol_;
	}
}

std::string GrammarSymbol::ToString() {
	return symbol_->ToString();
}
