#include <algorithm>
#include <functional>

#include "reader.h"
#include "utilities.h"
#include "debug.h"
#include "define.h"

FileReader::FileReader(const char* fileName, bool skipBlankline, bool appendNewline)
	: ifs_(fileName), lineNumber_(0), skipBlankline_(skipBlankline), appendNewline_(appendNewline) {

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
		if (ctext + 1 >= length) {
			Debug::LogError("buffer too small");
			return false;
		}

		buffer[ctext] = '\n';
		buffer[ctext + 1] = 0;
	}

	if (lineNumber != nullptr) {
		*lineNumber = lineNumber_;
	}

	return !!ifs_;
}

GrammarReader::GrammarReader(const char* source): source_(source) {
	ReadGrammars();
}

const GrammarTextContainer& GrammarReader::GetGrammars() const {
	return grammars_;
}

const char* GrammarReader::SplitGrammar(const char*& text) {
	text += strspn(text, ":\t\n ");
	if (*text == '|') {
		++text;
	}

	return std::find(text, text + strlen(text), '\t');
}

bool GrammarReader::GetLine(const char*& ptr, std::string& line) {
	const char* pos = strchr(ptr, '\n');
	if (pos == nullptr) {
		return false;
	}

	line.assign(ptr, pos);
	ptr = pos + 1;
	return true;
}

void GrammarReader::ReadGrammars() {
	GrammarText g;
	std::string line;
	int lineNumber = 1;

	const char* start = source_;
	for (; GetLine(start, line); ++lineNumber) {
		if (Utility::IsBlankText(line.c_str(), nullptr)) {
			if (!g.Empty()) {
				grammars_.push_back(g);
				g.Clear();
			}

			continue;
		}

		if (g.Empty()) {
			g.lhs = Utility::Trim(line);
			continue;
		}

		const char* ptr = line.c_str();
		const char* tabpos = SplitGrammar(ptr);
		if (*tabpos == 0) {
			Debug::LogError("missing \\t between production and action at line %d." + std::to_string(lineNumber));
			break;
		}

		g.productions.push_back(std::make_pair(Utility::Trim(std::string(ptr, tabpos)), Utility::Trim(std::string(tabpos))));
	}

	if (!g.Empty()) {
		grammars_.push_back(g);
	}
}

bool GrammarText::Empty() const {
	return lhs.empty();
}

void GrammarText::Clear() {
	lhs.clear(); productions.clear();
}
