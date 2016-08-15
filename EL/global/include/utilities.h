#pragma once

class Utility {
public:
	static int MakeWord(int low, int high);
	static int Loword(int dword);
	static int Highword(int dword);

private:
	Utility();
};

inline int Utility::Loword(int dword) {
	return dword & 0xffff;
}

inline int Utility::Highword(int dword) {
	return (dword >> 16) & 0xffff;
}

inline int Utility::MakeWord(int low, int high) {
	return (low & 0xffff) | ((high & 0xffff) << 16);
}
