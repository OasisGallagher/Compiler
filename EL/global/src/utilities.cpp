#include <cstdarg>

#include "debug.h"
#include "utilities.h"

#define HEADING_LENGTH			48
#define FORMAT_BUFFER_LENGTH	64

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

bool Utility::ParseInteger(const std::string& text, int* answer) {
	int integer = 0;

	for (const char* ptr = text.c_str(); *ptr != 0; ++ptr) {
		if (!Utility::IsDigit(*ptr)) {
			return false;
		}

		integer = integer * 10 + *ptr - '0';
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
	if (left == std::string::npos || right == std::string::npos) {
		return text;
	}

	return std::string(text.begin() + left, text.begin() + right + 1);
}

std::string Utility::Format(const char* format, ...) {
	char buffer[FORMAT_BUFFER_LENGTH] = { 0 };

	va_list ap;
	va_start(ap, format);
	int n = vsnprintf(buffer, FORMAT_BUFFER_LENGTH, format, ap);
	va_end(ap);

	Assert(n >= 0 && n < FORMAT_BUFFER_LENGTH, "format error");

	return buffer;
}

void Utility::Split(std::vector<std::string>& answer, const std::string& str, char seperator) {
	size_t from = 0, pos;
	do {
		pos = str.find(seperator, from);
		if (pos == std::string::npos) {
			answer.push_back(str.substr(from, pos));
			break;
		}

		answer.push_back(str.substr(from, pos - from));
		from = pos + 1;
	} while (true);
}
