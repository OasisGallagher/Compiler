#include <windows.h>

class Platform {
};

time_t G(const char* filePath) {
	HANDLE hFile = CreateFile(filePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    FILETIME ftWrite;
	SYSTEMTIME utcTime, localTime;

	GetFileTime(hFile, NULL, NULL, &ftWrite);
	FileTimeToSystemTime(&ftWrite, &utcTime);

	SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &localTime);

	tm temptm = { st.wSecond,
		st.wMinute,

		st.wHour,

		st.wDay,

		st.wMonth - 1,

		st.wYear - 1900,

		st.wDayOfWeek,

		0,

		0 };



	return mktime(&temptm);
}
