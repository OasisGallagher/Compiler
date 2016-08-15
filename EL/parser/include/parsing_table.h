#pragma once
#include <map>
#include <functional>
#include "grammar.h"

class ParsingTable {
private:
	typedef GrammarSymbol first_key_type;
	typedef GrammarSymbol second_key_type;
	typedef std::pair<first_key_type, second_key_type> key_type;
	typedef std::pair<GrammarSymbol, Condinate*> value_type;

	struct KeyComparer : std::binary_function < key_type, key_type, bool > {
		bool operator () (const key_type& lhs, const key_type& rhs) const;
	};

	typedef std::map<key_type, value_type, KeyComparer> Container;

public:
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;

public:
	value_type& at(const first_key_type& k1, const second_key_type& k2);
	value_type& operator () (const first_key_type& k1, const second_key_type& k2);
	
	iterator find(const first_key_type& k1, const second_key_type& k2);

	const_iterator begin() const;
	const_iterator end() const;

	std::string ToString() const;
private:
	Container cont_;
};
