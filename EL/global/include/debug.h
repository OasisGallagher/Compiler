#pragma once
#include <string>
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

class Debug {
public:
	static void Log(const std::string& text);
	static void LogWarning(const std::string& text);
	static void LogError(const std::string& text);

	static void Break(const std::string& message, const char* file, int line);

	static void EnableMemoryLeakCheck();
private:
	Debug();
	static void SetConsoleColor(int color);
};

#define Assert(expression, message)	\
	(void)((!!(expression)) || (Debug::Break(std::string(#expression) + ": " + message,  __FILE__, __LINE__), 0))
