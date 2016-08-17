#include <sstream>
#include <algorithm>
#include "grammar.h"
#include "debug.h"

std::string Condinate::ToString() const {
	std::ostringstream oss;

	const char* space = "";
	for (Condinate::const_iterator ite = begin(); ite != end(); ++ite) {
		oss << space << ite->ToString();
		space = " ";
	}

	return oss.str();
}

std::string CondinateContainer::ToString() const {
	std::ostringstream oss;

	char* space = "", *seperator = "";
	for (CondinateContainer::const_iterator ite = begin();
		ite != end(); ++ite) {
		space = "";

		oss << seperator;
		seperator = "|";

		oss << (*ite)->ToString();
	}

	return oss.str();
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
		// Add left recursion condinate to front.
		// TODO: O(n).
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
	struct CondinateComparer : public std::binary_function < Condinate*, Condinate*, bool > {
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

	std::sort(condinates_.begin(), condinates_.end(), CondinateComparer());
}

std::string Grammar::ToString() const {
	std::ostringstream oss;
	oss.width(22);
	oss.setf(std::ios::left);
	oss << left_.ToString();

	oss << condinates_.ToString();

	return oss.str();
}