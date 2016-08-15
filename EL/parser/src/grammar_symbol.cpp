#include <sstream>
#include "grammar_symbol.h"

GrammarSymbol GrammarSymbol::null = new Null();
GrammarSymbol GrammarSymbol::digit = new Digit();
GrammarSymbol GrammarSymbol::letter = new Letter();
GrammarSymbol GrammarSymbol::epsilon = new Epsilon();

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
	cont_[GrammarSymbol::null.ToString()] = GrammarSymbol::null;
	cont_[GrammarSymbol::digit.ToString()] = GrammarSymbol::digit;
	cont_[GrammarSymbol::letter.ToString()] = GrammarSymbol::letter;
	cont_[GrammarSymbol::epsilon.ToString()] = GrammarSymbol::epsilon;
}

GrammarSymbol GrammarSymbolContainer::AddSymbol(const std::string& text, bool terminal) {
	Container::iterator ite = cont_.find(text);
	GrammarSymbol ans;

	if (ite == cont_.end()) {
		if (terminal) {
			ans = new TerminalSymbol(text);
		}
		else {
			ans = new NonterminalSymbol(text);
		}

		cont_[text] = ans;
	}
	else {
		ans = ite->second;
	}

	return ans;
}

GrammarSymbolSetTable::GrammarSymbolSetTable() {
	cont_[GrammarSymbol::null].insert(GrammarSymbol::null);
	cont_[GrammarSymbol::digit].insert(GrammarSymbol::digit);
	cont_[GrammarSymbol::letter].insert(GrammarSymbol::letter);
	cont_[GrammarSymbol::epsilon].insert(GrammarSymbol::epsilon);
}

bool GrammarSymbolSetTable::IsBuildinSymbol(const GrammarSymbol& symbol) const {
	return symbol == GrammarSymbol::null
		|| symbol == GrammarSymbol::digit
		|| symbol == GrammarSymbol::letter
		|| symbol == GrammarSymbol::epsilon;
}

std::string GrammarSymbolSetTable::ToString() const {
	std::ostringstream oss;

	const char* newline = "";
	for (Container::const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		if (ite->first.SymbolType() == GrammarSymbolTerminal || IsBuildinSymbol(ite->first)) {
			continue;
		}

		oss << newline;
		newline = "\n";

		oss.width(12);
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

GrammarSymbolSet& GrammarSymbolSetTable::operator [](const GrammarSymbol& key) {
	Container::iterator ite = cont_.find(key);
	if (ite != cont_.end()) {
		return ite->second;
	}

	GrammarSymbolSet& ans = cont_[key];

	if (key.SymbolType() == GrammarSymbolTerminal) {
		ans.insert(key);
	}

	return ans;
}
