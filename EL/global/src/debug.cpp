#include <string>
#include <iostream>
#include <sstream>
#include <Windows.h>
#include "debug.h"

const int red = 4;
const int white = 7;
const int yellow = 6;

void Debug::Log(const std::string& text) {
	SetConsoleColor(white);
	std::cout << text << "\n";
}

void Debug::LogWarning(const std::string& text) {
	SetConsoleColor(yellow);
	std::cout << text << "\n";
	SetConsoleColor(white);
}

void Debug::LogError(const std::string& text) {
	SetConsoleColor(red);
	std::cout << text << "\n";
	SetConsoleColor(white);
}

void Debug::SetConsoleColor(int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
}

void Debug::Break(const char* message, const char* file, int line) {
	std::ostringstream oss;
	oss << message << "\n";
	oss << "at " << file << ":" << line;
	Debug::LogError(oss.str());

	abort();
}
