#pragma once
#include <fstream>

class FileReader {
public:
	FileReader(const char* path);
	~FileReader();

public:
	bool ReadLine(char* buffer, size_t length, int* lineNumber);

private:
	bool IsBlankLine(const char* text);

private:
	int lineNumber_;
	std::ifstream ifs_;
};
