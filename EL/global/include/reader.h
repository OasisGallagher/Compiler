#pragma once
#include <vector>
#include <fstream>

class FileReader {
public:
	FileReader(const char* fileName, bool skipBlankline, bool appendNewline);
	~FileReader();

public:
	bool ReadLine(char* buffer, size_t length, int* lineNumber);

private:
	bool skipBlankline_;
	bool appendNewline_;
	int lineNumber_;
	std::ifstream ifs_;
};

typedef std::pair<std::string, std::string> ProductionText;

struct GrammarText {
	std::string lhs;
	typedef std::vector<ProductionText> ProductionTextContainer;
	ProductionTextContainer productions;

	void Clear();
	bool Empty() const;
};

typedef std::vector<GrammarText> GrammarTextContainer;

class GrammarReader {
public:
	GrammarReader(const char* file);

public:
	const GrammarTextContainer& GetGrammars() const;

private:
	void ReadGrammars();
	const char* SplitGrammar(char*& text);

private:
	FileReader fileReader_;
	GrammarTextContainer grammars_;
};
