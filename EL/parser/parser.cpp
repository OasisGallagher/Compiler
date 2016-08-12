#include <cassert>
#include <sstream>
#include <algorithm>
#include <functional>
#include "../scanner/scanner.h"
#include "parser.h"
#include "constants.h"
#include "debug.h"

GrammarSymbol GrammarSymbol::epsilon = new Epsilon();
GrammarSymbol GrammarSymbol::alphabet = new Alphabet();
GrammarSymbol GrammarSymbol::digits = new Digits();

GrammarSymbolContainer::GrammarSymbolContainer() {
	cont_[GrammarSymbol::epsilon.ToString()] = GrammarSymbol::epsilon;
	cont_[GrammarSymbol::alphabet.ToString()] = GrammarSymbol::alphabet;
	cont_[GrammarSymbol::digits.ToString()] = GrammarSymbol::digits;
}

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

Grammar::Grammar() {
}

Grammar::Grammar(const GrammarSymbol& left) 
	: left_(left) {
}

Grammar::~Grammar() {
	for (CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		delete *ite;
	}
}

void Grammar::SetLeft(const GrammarSymbol& symbol) {
	left_ = symbol;
}

const GrammarSymbol& Grammar::GetLeft() const {
	return left_;
}

Condinate* Grammar::AddCondinate() {
	Condinate* ans = new Condinate();
	condinates_.push_back(ans);
	return ans;
}

const CondinateContainer& Grammar::GetCondinates() const {
	return condinates_;
}

void Grammar::SortCondinates() {
	CondinateContainer::iterator pos = condinates_.begin();
	for(CondinateContainer::iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		Condinate* cond = *ite;
		if(cond->front() == left_) {
			std::iter_swap(pos++, ite);
		}
	}
}

std::string Grammar::ToString() const {
	std::ostringstream os;
	os << left_.ToString();
	os << " : ";

	char* space = "", *xor = "";
	for (CondinateContainer::const_iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		space = "";

		os << xor;
		xor = "|";

		for (Condinate::iterator ite2 = (*ite)->begin(); 
			ite2 != (*ite)->end(); ++ite2) {
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
	for (GrammarContainer::iterator ite = grammars_.begin(); 
		ite != grammars_.end(); ++ite) {
		delete *ite;
	}
}

bool GrammarParser::Parse(const char* productions[], int count) {
	GrammarSymbolContainer cont;
	LineScanner lineScanner;
	for (int i = 0; i < count; ++i) {
		lineScanner.SetText(productions[i]);
		if (!ParseProductions(&lineScanner, &cont)) {
			return false;
		}
	}

	return ParseGrammar();
}

bool GrammarParser::ParseGrammar() {
	RemoveLeftRecursion();
	return true;
}

void GrammarParser::RemoveLeftRecursion() {
	std::for_each(grammars_.begin(), grammars_.end(), std::mem_fun(&Grammar::SortCondinates));
	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ) {
		if(HandleLeftRecursion(*ite, &newGrammars)) {
			ite = grammars_.erase(ite);
			grammars_.insert(ite, newGrammars.begin(), newGrammars.end());
		}
		else {
			++ite;
		}
	}
}

bool GrammarParser::HandleLeftRecursion(Grammar* g, GrammarContainer* newGrammars) {
	const GrammarSymbol& left = g->GetLeft();
	const CondinateContainer& condinates = g->GetCondinates();

	CondinateContainer::const_iterator pos = condinates.begin();
	for (; pos != condinates.end(); ++pos) {
		if ((*pos)->front() != left) {
			break;
		}
	}

	if (pos == condinates.begin()) {
		return false;
	}

	Assert(pos != condinates.end(), "invalid production");
	Grammar* grammar = new Grammar(left);

	GrammarSymbol left2 = new NonterminalSymbol((left.ToString() + "_2").c_str());
	Grammar* grammar2 = new Grammar(left2);

	for (CondinateContainer::const_iterator ite = pos; ite != condinates.end(); ++ite) {
		Condinate* cond = grammar->AddCondinate();
		cond->insert(cond->end(), (*ite)->begin(), (*ite)->end());
		cond->push_back(left2);
	}

	for (CondinateContainer::const_iterator ite = condinates.begin(); ite != pos; ++ite) {
		Condinate* cond = grammar2->AddCondinate();
		cond->insert(cond->end(), (*ite)->begin() + 1, (*ite)->end());
		cond->push_back(left2);
	}

	grammar2->AddCondinate()->push_back(GrammarSymbol::epsilon);

	newGrammars->push_back(grammar);
	newGrammars->push_back(grammar2);

	Debug::Log(grammar->ToString());
	Debug::Log(grammar2->ToString());

	return true;
}

bool GrammarParser::ParseProductions(LineScanner* lineScanner, GrammarSymbolContainer* symbols) {
	char token[Constants::kMaxTokenCharacters];

	ScannerTokenType tokenType = lineScanner->GetToken(token);
	Assert(tokenType != ScannerTokenEndOfFile, "invalid production. missing left part.");

	Grammar* grammar = new Grammar(symbols->AddSymbol(token, false));

	Condinate* cond = grammar->AddCondinate();

	lineScanner->GetToken(token);
	Assert(strcmp(token, ":") == 0, "invalid production. missing \":\".");

	for (; (tokenType = lineScanner->GetToken(token)) != ScannerTokenEndOfFile; ) {
		if (tokenType == ScannerTokenXor) {
			Assert(!cond->empty(), "empty production");
			cond = grammar->AddCondinate();
			continue;
		}

		cond->push_back(symbols->AddSymbol(token, IsTerminal(token)));
	}
	
	grammars_.push_back(grammar);

	return true;
}

bool GrammarParser::IsTerminal(const char* token) {
	return *token == '$';
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

bool GrammarSymbol::operator == (const GrammarSymbol& other) const {
	return symbol_ == other.symbol_;
}

bool GrammarSymbol::operator != (const GrammarSymbol& other) const {
	return symbol_ != other.symbol_;
}

std::string GrammarSymbol::ToString() const {
	return symbol_->ToString();
}
