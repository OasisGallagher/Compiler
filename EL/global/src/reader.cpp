#include <algorithm>
#include <functional>

#include "debug.h"
#include "reader.h"
#include "utilities.h"
#include "constants.h"

FileReader::FileReader(const char* path, bool skipBlankline, bool appendNewline)
	: ifs_(path), lineNumber_(0), skipBlankline_(skipBlankline), appendNewline_(appendNewline) {

}

FileReader::~FileReader() {
	ifs_.close();
}

bool FileReader::ReadLine(char* buffer, size_t length, int* lineNumber) {
	for (; ifs_.getline(buffer, length);) {
		++lineNumber_;
		if (!skipBlankline_ || !Utility::IsBlankText(buffer)) {
			break;
		}
	}

	if (appendNewline_ && !ifs_.eof()) {
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

ProductionReader::ProductionReader(const char* file) : fileReader_(file, false, false) {
	ReadProductions();
}

const ProductionReader::ProducitonContainer& ProductionReader::GetProductions() const {
	return grammars_;
}

void ProductionReader::ReadProductions() {
	char buffer[MAX_LINE_CHARACTERS];
	std::string g;
	for (; fileReader_.ReadLine(buffer, MAX_LINE_CHARACTERS, nullptr);) {
		if (Utility::IsBlankText(buffer)) {
			if (!g.empty()) {
				grammars_.push_back(g);
				g.clear();
			}

			continue;
		}

		g += buffer;
	}

	if (!g.empty()) {
		grammars_.push_back(g);
	}
}
