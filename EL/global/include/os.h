#include <ctime>
#if PLATFORM_WINDOWS
#include "os_windows.h"
#elif PLATFORM_LINUX
#include "os_linux.h"
#else
#error "invalid platform"
#endif

enum Color {
	Red,
	White,
	Green,
	Yellow,
};

class OS {
public:
	static void Break(const char* message);
	static void SetConsoleColor(Color color);
	static void EnableMemoryLeakCheck();
	static time_t GetFileLastWriteTime(const char* filePath);

private:
	OS();
};
