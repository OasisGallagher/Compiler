#pragma once
#include <fstream>

class FileReader {
public:
	FileReader(const char* path) :ifs_(path) {
	}

	~FileReader() {
		ifs_.close();
	}

public:
	bool ReadLine(char* buffer, size_t length) {
		return !!ifs_.getline(buffer, length);
	}

private:
	std::ifstream ifs_;
};
