#pragma once
#include <stack>
#include <string>
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

class Debug {
public:
	static void Log(const std::string& text);
	static void LogWarning(const std::string& text);
	static void LogError(const std::string& text);

	static void Break(const std::string& expression, const std::string& message, const char* file, int line);

	static void EnableMemoryLeakCheck();

	static void StartSample(const std::string& text);
	static void EndSample();

private:
	Debug();
	static void SetConsoleColor(int color);
	static std::string GetTime();

private:
	static std::stack<std::string> samples_;
};

#define Verify(expression, message)	\
	(void)((!!(expression)) || (Debug::Break(#expression, message,  __FILE__, __LINE__), 0))

#if _DEBUG
#define Assert(expression, message)	\
	Verify(expression, message)
#else
	(void)0
#endif
