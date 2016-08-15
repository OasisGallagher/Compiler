#include <cassert>
#include <sstream>
#include <algorithm>
#include <functional>
#include "scanner.h"
#include "parser.h"
#include "constants.h"
#include "debug.h"
#include "utilities.h"

GrammarSymbol GrammarSymbol::epsilon = new Epsilon();
GrammarSymbol GrammarSymbol::letter = new Alphabet();
GrammarSymbol GrammarSymbol::digit = new Digits();

GrammarSymbolContainer::GrammarSymbolContainer() {
	cont_[GrammarSymbol::epsilon.ToString()] = GrammarSymbol::epsilon;
	cont_[GrammarSymbol::letter.ToString()] = GrammarSymbol::letter;
	cont_[GrammarSymbol::digit.ToString()] = GrammarSymbol::digit;
}

GrammarSymbol GrammarSymbolContainer::AddSymbol(const std::string& text, bool terminal) {
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

void Grammar::AddCondinate(const Condinate& cond) {
	Assert(!cond.empty(), "empty condinate");
	Condinate* ptr = new Condinate(cond);
	if (ptr->front() == left_) {
		// TODO.
		condinates_.insert(condinates_.begin(), ptr);
	}
	else {
		condinates_.push_back(ptr);
	}
}

const CondinateContainer& Grammar::GetCondinates() const {
	return condinates_;
}

void Grammar::SortCondinates() {
	struct CondinateComparer : public std::binary_function<Condinate*, Condinate*, bool> {
		bool operator () (const Condinate* lhs, const Condinate* rhs) const {
			Condinate::const_iterator lite = lhs->begin(), rite = rhs->begin();
			for (; lite != lhs->end() && rite != rhs->end() && *lite == *rite;) {
				++lite, ++rite;
			}

			if (lite == lhs->end()) {
				return false;
			}

			if (rite == rhs->end()) {
				return true;
			}

			return *lite > *rite;
		}
	};

	CondinateComparer comparer;
	std::sort(condinates_.begin(), condinates_.end(), comparer);
}

std::string Grammar::ToString() const {
	std::ostringstream os;
	os << left_.ToString();
	os << " : ";

	char* space = "", *seperator = "";
	for (CondinateContainer::const_iterator ite = condinates_.begin();
		ite != condinates_.end(); ++ite) {
		space = "";

		os << seperator;
		seperator = "|";

		for (Condinate::iterator ite2 = (*ite)->begin(); 
			ite2 != (*ite)->end(); ++ite2) {
			os << space << ite2->ToString();
			space = " ";
			std::string tmp = os.str();
		}
	}

	return os.str();
}

Language::Language() {
}

Language::~Language() {
	for (GrammarContainer::iterator ite = grammars_.begin(); 
		ite != grammars_.end(); ++ite) {
		delete *ite;
	}
}

bool Language::SetGrammar(const char* productions[], int count) {
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

bool Language::ParseGrammar() {
	RemoveLeftRecursion();
	LeftFactoring();

	std::string text;
	const char* newline = "";
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		text += newline;
		newline = "\n";
		text += (*ite)->ToString();
	}

	Debug::Log("Grammars:");
	Debug::Log(text);

	return true;
}

void Language::RemoveLeftRecursion() {
	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ) {
		if(RemovImmidiateLeftRecursion(*ite, &newGrammars)) {
			ite = grammars_.erase(ite);
			grammars_.insert(ite, newGrammars.begin(), newGrammars.end());
			newGrammars.clear();
		}
		else {
			++ite;
		}
	}
}

bool Language::RemovImmidiateLeftRecursion(Grammar* g, GrammarContainer* newGrammars) {
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

	GrammarSymbol left2 = new NonterminalSymbol((left.ToString() + "_2"));
	Grammar* grammar2 = new Grammar(left2);
	Condinate cond;

	for (CondinateContainer::const_iterator ite = pos; ite != condinates.end(); ++ite) {
		cond.insert(cond.end(), (*ite)->begin(), (*ite)->end());
		cond.push_back(left2);

		grammar->AddCondinate(cond);
		cond.clear();
	}

	for (CondinateContainer::const_iterator ite = condinates.begin(); ite != pos; ++ite) {
		cond.insert(cond.end(), (*ite)->begin() + 1, (*ite)->end());
		cond.push_back(left2);

		grammar2->AddCondinate(cond);
		cond.clear();
	}

	cond.push_back(GrammarSymbol::epsilon);
	grammar2->AddCondinate(cond);

	newGrammars->push_back(grammar);
	newGrammars->push_back(grammar2);

	return true;
}

int Language::LongestCommonPrefix(const Condinate* first, const Condinate* second) {
	int size = std::min(first->size(), second->size());
	int ans = 0;
	for (int i = 0; i < size; ++i) {
		if (first->at(i) != second->at(i)) {
			break;
		}

		++ans;
	}

	return ans;
}

void Language::LeftFactoring() {
	std::for_each(grammars_.begin(), grammars_.end(), std::mem_fun(&Grammar::SortCondinates));

	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end();) {
		if (LeftFactoringOnGrammar(*ite, &newGrammars)) {
			delete *ite;
			ite = grammars_.erase(ite);
			grammars_.insert(ite, newGrammars.begin(), newGrammars.end());
			newGrammars.clear();
		}
		else {
			++ite;
		}
	}
}

bool Language::CalculateFactorLength(Grammar* g, int* range, int* length) {
	const CondinateContainer& cond = g->GetCondinates();
	int maxlength = 0;
	int from = 0, index = 1;
	for (; index < (int)cond.size(); ++index) {
		int length = LongestCommonPrefix(cond[index - 1], cond[index]);

		if (length < maxlength) {
			break;
		}

		if (length > maxlength) {
			from = index - 1;
			maxlength = length;
		}
	}

	if (maxlength == 0) {
		return false;
	}

	*length = maxlength;
	*range = Utility::MakeWord(from, index);
	return true;
}

bool Language::LeftFactoringOnGrammar(Grammar* g, GrammarContainer* newGrammars) {
	int range, length, nsindex = 1;
	Grammar* seed = g;
	for (; CalculateFactorLength(g, &range, &length);) {
		int from = Utility::Loword(range), to = Utility::Highword(range);
		Grammar* grammar = new Grammar(g->GetLeft());

		GrammarSymbol left2 = new NonterminalSymbol((g->GetLeft().ToString() + "_" + std::to_string(++nsindex)));

		Condinate cond;
		const CondinateContainer& oldCondinates = g->GetCondinates();
		Condinate::iterator first = oldCondinates[from]->begin();
		Condinate::iterator last = first;
		std::advance(last, length);
		cond.insert(cond.end(), first, last);

		cond.push_back(left2);
		grammar->AddCondinate(cond);

		for (int i = 0; i < from; ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		for (int i = to; i < (int)oldCondinates.size(); ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		Grammar* grammar2 = new Grammar(left2);
		for (int i = from; i < to; ++i) {
			Assert(length <= (int)oldCondinates[i]->size(), "logic error");
			first = oldCondinates[i]->begin();
			std::advance(first, length);
			cond.clear();
			if (first != oldCondinates[i]->end()) {
				cond.insert(cond.end(), first, oldCondinates[i]->end());
				grammar2->AddCondinate(cond);
			}
			else {
				cond.push_back(GrammarSymbol::epsilon);
				grammar2->AddCondinate(cond);
			}
		}

		Debug::Log(g->ToString());

		if (g != seed) {
			delete g;
		}

		//newGrammars->push_back(grammar);
		newGrammars->push_back(grammar2);

		g = grammar;
		g->SortCondinates();

		Debug::Log(grammar->ToString());
		Debug::Log(grammar2->ToString());
	}

	if (g != seed) {
		newGrammars->push_back(g);
	}

	return true;
}

bool Language::ParseProductions(LineScanner* lineScanner, GrammarSymbolContainer* symbols) {
	char token[Constants::kMaxTokenCharacters];

	ScannerTokenType tokenType = lineScanner->GetToken(token);
	Assert(tokenType != ScannerTokenEndOfFile, "invalid production. missing left part.");

	Grammar* grammar = new Grammar(symbols->AddSymbol(token, false));

	Condinate cond;

	lineScanner->GetToken(token);
	Assert(strcmp(token, ":") == 0, "invalid production. missing \":\".");

	for (; (tokenType = lineScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		if (tokenType == ScannerTokenSeperator) {
			Assert(!cond.empty(), "empty production");
			grammar->AddCondinate(cond);
			cond.clear();
			continue;
		}

		cond.push_back(symbols->AddSymbol(token, IsTerminal(token)));
	}

	grammar->AddCondinate(cond);
	
	grammars_.push_back(grammar);

	return true;
}

bool Language::IsTerminal(const char* token) {
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

bool GrammarSymbol::operator < (const GrammarSymbol& other) const {
	return symbol_ < other.symbol_;
}

bool GrammarSymbol::operator > (const GrammarSymbol& other) const {
	return symbol_ > other.symbol_;
}

std::string GrammarSymbol::ToString() const {
	return symbol_->ToString();
}
