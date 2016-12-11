#pragma once
#include <map>
#include <string>
#include <fstream>

class Utility {
public:
	static int MakeDword(int low, int high);
	static int Loword(int dword);
	static int Highword(int dword);

	static bool IsTerminal(const std::string& token);

	static bool IsBlankText(const char* text, const char** pos = nullptr);

	// TODO: 只解析简单的正整数.
	static bool ParseInteger(const char* text, int* answer = nullptr);

	static std::string Trim(const std::string& text);

	static bool IsDigit(int c);
	static bool IsLetter(int c);

	template <class Iterator, class Comp>
	static Iterator FindGroup(Iterator first, Iterator last, const Comp& comp);

	template <class Iterator>
	static std::string Concat(Iterator first, Iterator last, const char* seperator = " ");

	static std::string Heading(const std::string& text);
private:
	Utility();
};

inline int Utility::Loword(int dword) {
	return dword & 0xffff;
}

inline int Utility::Highword(int dword) {
	return (dword >> 16) & 0xffff;
}

inline int Utility::MakeDword(int low, int high) {
	return (low & 0xffff) | ((high & 0xffff) << 16);
}

inline bool Utility::IsTerminal(const std::string& token) {
	return token.front() != '$';
}

inline bool Utility::IsDigit(int c) {
	return isdigit(c) != 0;
}

inline bool Utility::IsLetter(int c) {
	return (isalpha(c) != 0) || c == '_' || c == '$';
}

template <class Iterator>
std::string Utility::Concat(Iterator first, Iterator last, const char* seperator) {
	const char* sep = "";
	std::ostringstream oss;

	for (; first != last; ++first) {
		oss << sep;
		oss << first->ToString();
		sep = seperator;
	}

	return oss.str();
}

template <class Iterator, class Comp>
Iterator Utility::FindGroup(Iterator first, Iterator last, const Comp& comp) {
	Iterator current = first;
	for (; ++current != last;) {
		if (!comp(*first, *current)) {
			return current;
		}
	}

	return current;
}
