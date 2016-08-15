#include "debug.h"
#include "utilities.h"

std::string Utility::Heading(const std::string& text, int length) {
	Assert(length >= (int)text.length(), "invalid parameter");
	int left = (length - text.length()) / 2;
	std::string ans(length, '=');
	for (int i = 0; i < (int)text.length(); ++i) {
		ans[left++] = text[i];
	}

	return ans;
}
