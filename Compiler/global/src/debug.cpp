#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "debug.h"
#include "utilities.h"

int Debug::length_ = 0;
std::stack<std::string> Debug::samples_;

std::ofstream debug("main/debug/debug.txt");

void Debug::Log(const std::string& text) {
	OS::SetConsoleColor(White);
	std::cout << text << "\n";
	debug << text << "\n";
}

void Debug::LogWarning(const std::string& text) {
	OS::SetConsoleColor(Yellow);
	std::cout << "[W] " << text << "\n";
	debug << "[W] " << text << "\n";
	OS::SetConsoleColor(White);
}

void Debug::LogError(const std::string& text) {
	OS::SetConsoleColor(Red);
	std::cout << "[E] " << text << "\n";
	debug << "[E] " << text << "\n";
	OS::SetConsoleColor(White);
}

std::string Debug::Now() {
	time_t now = time(nullptr);
	tm* ptr = localtime(&now);
	return Utility::Format("%02d:%02d:%02d", ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
}

void Debug::Break(const std::string& expression, const std::string& message, const char* file, int line) {
	std::ostringstream oss;
	oss << expression + ":\n" + message << "\n";
	oss << "at " << file << ":" << line;
	Debug::LogError(oss.str());

	OS::Break(oss.str().c_str());
}

void Debug::EnableMemoryLeakCheck() {
	OS::EnableMemoryLeakCheck();
}

void Debug::StartProgress() {
	length_ = 0;
	OS::SetConsoleColor(Green);
}

void Debug::LogProgress(const char* text, int current, int total) {
	std::cout << std::string(length_, '\b');

	std::string log = Utility::Format("%s (%d/%d).", text, current, total);
	std::cout << log;
	length_ = log.length();
}

void Debug::EndProgress() {
	std::cout << std::string(length_, '\b');
	OS::SetConsoleColor(White);
}

void Debug::StartSample(const std::string& text) {
	Debug::Log("\"" + text + "\" at " + Now() + ".");

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
