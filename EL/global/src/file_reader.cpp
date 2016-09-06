#include <algorithm>
#include <functional>

#include "debug.h"
#include "file_reader.h"

FileReader::FileReader(const char* path) :ifs_(path), lineNumber_(0) {

}

FileReader::~FileReader() {
	ifs_.close();
}

bool FileReader::IsBlankLine(const char* text) {
	for (; *text != 0; ++text) {
		if (*text != ' ' && *text != '\t' && *text != '\n') {
			return false;
		}
	}

	return true;
}

bool FileReader::ReadLine(char* buffer, size_t length, int* lineNumber) {
	for (; ifs_.getline(buffer, length);) {
		++lineNumber_;
		if (!IsBlankLine(buffer)) {
			break;
		}
	}

	if (!ifs_.eof()) {
		size_t ctext = strlen(buffer);
		Assert(ctext + 1 < length, "buffer too small");
		buffer[ctext] = '\n';
		buffer[ctext + 1] = 0;
	}

	if (lineNumber != nullptr) {
		*lineNumber = lineNumber_;
	}

	return !!ifs_;
}
