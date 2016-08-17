#include <sstream>
#include "parsing_table.h"

bool ParsingTable::KeyComparer::operator()(const key_type& lhs, const key_type& rhs) const {
	if (lhs.first == rhs.first) {
		return lhs.second < rhs.second;
	}

	return lhs.first < rhs.first;
}

ParsingTable::value_type& ParsingTable::at(const first_key_type& k1, const second_key_type& k2) {
	return cont_[std::make_pair(k1, k2)];
}

ParsingTable::value_type& ParsingTable::operator()(const first_key_type& k1, const second_key_type& k2) {
	return cont_[std::make_pair(k1, k2)];
}

ParsingTable::iterator ParsingTable::find(const first_key_type& k1, const second_key_type& k2) {
	return cont_.find(std::make_pair(k1, k2));
}

ParsingTable::const_iterator ParsingTable::begin() const {
	return cont_.begin();
}

ParsingTable::const_iterator ParsingTable::end() const {
	return cont_.end();
}

std::string ParsingTable::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (Container::const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << value.first.ToString() << " : ";

		oss << value.second->ToString();
	}

	return oss.str();
}