#if PLATFORM_LINUX

#include "os.h"

void OS::Break(const char* message) {
}

void OS::SetConsoleColor(Color color) {
}

void OS::EnableMemoryLeakCheck() {
}

time_t OS::GetFileLastWriteTime(const char* filePath) {
	return 0;
}

bool OS::Prompt(const char* message) {
	return true;
}

#endif
