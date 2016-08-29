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
