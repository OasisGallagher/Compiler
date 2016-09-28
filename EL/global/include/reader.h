#pragma once
#include <vector>
#include <fstream>

class FileReader {
public:
	FileReader(const char* path, bool skipBlankline, bool appendNewline);
	~FileReader();

public:
	bool ReadLine(char* buffer, size_t length, int* lineNumber);

private:
	bool skipBlankline_;
	bool appendNewline_;
	int lineNumber_;
	std::ifstream ifs_;
};

typedef std::pair<std::string, std::string> ProductionDef;

struct GrammarDef {
	std::string lhs;
	typedef std::vector<ProductionDef> ProductionDefContainer;
	ProductionDefContainer productions;

	void Clear();
	bool Empty() const;
};

typedef std::vector<GrammarDef> GrammarDefContainer;

class GrammarReader {
public:
	GrammarReader(const char* file);

public:
	const GrammarDefContainer& GetGrammars() const;

private:
	void ReadGrammars();
	const char* SplitGrammar(char*& text);

private:
	FileReader fileReader_;
	GrammarDefContainer grammars_;
};
