#include "debug.h"
#include "table.h"
#include "utilities.h"

Sym::Sym(const std::string& text) {
	value_ = text;
}

std::string Sym::ToString() const {
	return value_;
}

Constant::Constant(const std::string& text) {
	int integer = 0;
	bool valid = Utility::ParseInteger(text.c_str(), &integer);
	Assert(valid, "invalid integer " + text);
	value_ = integer;
}

std::string Constant::ToString() const {
	return std::to_string(value_);
}

Literal::Literal(const std::string& text) {
	value_ = text;
}

std::string Literal::ToString() const {
	return value_;
}
