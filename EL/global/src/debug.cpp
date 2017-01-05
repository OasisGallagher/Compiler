#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>

#include "debug.h"
#include "utilities.h"

std::stack<std::string> Debug::samples_;

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

std::string Debug::GetTime() {
	return "";
}

void Debug::Break(const std::string& expression, const std::string& message, const char* file, int line) {
	std::ostringstream oss;
	oss << expression + ":\n" + message << "\n";
	oss << "at " << file << ":" << line;
	Debug::LogError(oss.str());

	MessageBox(NULL, message.c_str(), "Assertion Failed", MB_OK | MB_ICONERROR);
	__asm int 3
}

void Debug::EnableMemoryLeakCheck() {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

void Debug::StartSample(const std::string& text) {
	clock_t now = clock();
	samples_.push(text + "@" + std::to_string(now));
}

void Debug::EndSample() {
	std::string samp = samples_.top();
	int pos = samp.rfind('@');
	samples_.pop();
	clock_t elapsed = clock() - atol(samp.c_str() + pos + 1);
	samp[pos] = 0;
	Debug::Log(Utility::Format("\"%s\" costs %.2f seconds.", samp.c_str(), ((float)elapsed / CLOCKS_PER_SEC)));
}
