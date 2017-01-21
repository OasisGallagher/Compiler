#if PLATFORM_WINDOWS
#include <windows.h>

#include "os.h"

void OS::Break(const char* message) {
	MessageBox(NULL, message, "Assertion Failed", MB_OK | MB_ICONERROR);
	__asm int 3
}

void OS::SetConsoleColor(Color color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int ncolor = 0;
	switch (color) {
	case Red:
		ncolor = FOREGROUND_INTENSITY | FOREGROUND_RED;
		break;
	case White:
		ncolor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		break;
	case Green:
		ncolor = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
		break;
	case Yellow:
		ncolor = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
		break;
	}

	SetConsoleTextAttribute(handle, ncolor);
}

void OS::EnableMemoryLeakCheck() {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

time_t OS::GetFileLastWriteTime(const char* filePath) {
	HANDLE hFile = CreateFile(filePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	FILETIME ftWrite;
	SYSTEMTIME utcTime, localTime;

	GetFileTime(hFile, NULL, NULL, &ftWrite);
	FileTimeToSystemTime(&ftWrite, &utcTime);

	SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &localTime);

	tm temptm = {
		localTime.wSecond,
		localTime.wMinute,
		localTime.wHour,
		localTime.wDay,
		localTime.wMonth - 1,
		localTime.wYear - 1900,
		localTime.wDayOfWeek,
	};

	return mktime(&temptm);
}

#endif
