#include <sstream>
#include "parsing_table.h"

std::string ParsingTable::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << value.first.ToString();
		
		if (value.second != nullptr) {
			oss << " : ";
			oss << value.second->ToString();
		}
	}

	return oss.str();
}
