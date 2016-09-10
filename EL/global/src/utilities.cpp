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

bool Utility::IsBlankText(const char* text) {
	for (; *text != 0; ++text) {
		if (*text != ' ' && *text != '\t' && *text != '\n') {
			return false;
		}
	}

	return true;
}
