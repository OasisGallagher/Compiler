#include <sstream>
#include "grammar_symbol.h"

GrammarSymbol GrammarSymbol::zero = new Zero();
GrammarSymbol GrammarSymbol::synch = new Synch();
GrammarSymbol GrammarSymbol::number = new Number();
GrammarSymbol GrammarSymbol::string = new String();
GrammarSymbol GrammarSymbol::epsilon = new Epsilon();
GrammarSymbol GrammarSymbol::identifier = new Identifier();

GrammarSymbol::GrammarSymbol()
	: symbol_(nullptr) {
}

GrammarSymbol::GrammarSymbol(GrammarSymbolBase* symbol)
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

GrammarSymbol::operator bool() const {
	return symbol_ != nullptr;
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

bool GrammarSymbol::operator >(const GrammarSymbol& other) const {
	return symbol_ > other.symbol_;
}

GrammarSymbolType GrammarSymbol::SymbolType() const {
	return symbol_->SymbolType();
}

bool GrammarSymbol::Match(const std::string& text) const {
	return symbol_->Match(text);
}

std::string GrammarSymbol::ToString() const {
	return symbol_->ToString();
}

GrammarSymbolContainer::GrammarSymbolContainer() {
	insert(std::make_pair(GrammarSymbol::zero.ToString(), GrammarSymbol::zero));
	insert(std::make_pair(GrammarSymbol::number.ToString(), GrammarSymbol::number));
	insert(std::make_pair(GrammarSymbol::string.ToString(), GrammarSymbol::string));
	insert(std::make_pair(GrammarSymbol::epsilon.ToString(), GrammarSymbol::epsilon));
	insert(std::make_pair(GrammarSymbol::identifier.ToString(), GrammarSymbol::identifier));
}

GrammarSymbol GrammarSymbolContainer::AddSymbol(const std::string& text, bool terminal) {
	iterator ite = find(text);
	GrammarSymbol ans;

	if (ite == end()) {
		if (terminal) {
			ans = new TerminalSymbol(text);
		}
		else {
			ans = new NonterminalSymbol(text);
		}

		insert(std::make_pair(text, ans));
	}
	else {
		ans = ite->second;
	}

	return ans;
}

std::string GrammarSymbolSetTable::ToString() const {
	std::ostringstream oss;

	const char* newline = "";
	for (const_iterator ite = begin(); ite != end(); ++ite) {
		if (ite->first.SymbolType() == GrammarSymbolTerminal) {
			continue;
		}

		oss << newline;
		newline = "\n";

		oss.width(22);
		oss.setf(std::ios::left);

		oss << ite->first.ToString();
		oss << "{";

		const char* seperator = "";
		for (GrammarSymbolSet::const_iterator ite2 = ite->second.begin(); ite2 != ite->second.end(); ++ite2) {
			oss << seperator;
			seperator = " ";
			oss << ite2->ToString();
		}

		oss << "}";
	}

	return oss.str();
}
