#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include "debug.h"

clock_t Timer::start_;

const int red = 12;
const int white = 7;
const int yellow = 14;

std::ofstream output("main/debug/debug.txt");

void Debug::Log(const std::string& text) {
	SetConsoleColor(white);
	std::cout << text << "\n";
	output << text << "\n";
}

void Debug::LogWarning(const std::string& text) {
	SetConsoleColor(yellow);
	std::cout << "[W] " << text << "\n";
	output << "[W] " << text << "\n";
	SetConsoleColor(white);
}

void Debug::LogError(const std::string& text) {
	SetConsoleColor(red);
	std::cout << "[E] " << text << "\n";
	output << "[E] " << text << "\n";
	SetConsoleColor(white);
}

void Debug::SetConsoleColor(int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
}

void Debug::Break(const std::string& message, const char* file, int line) {
	std::ostringstream oss;
	oss << message << "\n";
	oss << "at " << file << ":" << line;
	Debug::LogError(oss.str());
	__asm int 3;
}

void Debug::EnableMemoryLeakCheck() {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

void Timer::Start() {
	start_ = clock();
}

float Timer::Stop() {
	return ((float)clock() - start_) / CLOCKS_PER_SEC;
}
