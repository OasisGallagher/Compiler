#pragma once
#include <string>

class Debug {
public:
	static void Log(const std::string& text);
	static void LogWarning(const std::string& text);
	static void LogError(const std::string& text);

	static void Break(const char* message, const char* file, int line);

private:
	Debug();
	static void SetConsoleColor(int color);
};

#define Assert(expression, message)	\
	(void)((!!(expression)) || (Debug::Break(#expression ": " message,  __FILE__, __LINE__), 0))
