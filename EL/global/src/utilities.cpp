#include "debug.h"
#include "utilities.h"

#define HEADING_LENGTH		48

std::string Utility::Heading(const std::string& text) {
	Assert(HEADING_LENGTH >= (int)text.length(), "invalid parameter");
	int left = (HEADING_LENGTH - text.length()) / 2;
	std::string ans(HEADING_LENGTH, '=');
	for (int i = 0; i < (int)text.length(); ++i) {
		ans[left++] = text[i];
	}

	return ans;
}

bool Utility::IsBlankText(const char* text, const char** pos) {
	for (; *text != 0; ++text) {
		if (*text != ' ' && *text != '\t' && *text != '\n') {
			if (pos != nullptr) {
				*pos = text;
			}

			return false;
		}
	}

	return true;
}

bool Utility::ParseInteger(const char* text, int* answer) {
	int integer = 0;
	for (; *text != 0; ++text) {
		if (!isdigit(*text)) {
			return false;
		}

		integer = integer * 10 + *text - '0';
	}

	if (answer != nullptr) {
		*answer = integer;
	}

	return true;
}

std::string Utility::Trim(const std::string& text) {
	const char* whitespaces = " \t";
	size_t left = text.find_first_not_of(whitespaces);
	size_t right = text.find_last_not_of(whitespaces);
	
	return std::string(text.begin() + left, text.begin() + right + 1);
}
