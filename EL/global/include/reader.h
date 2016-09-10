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

class ProductionReader {
public:
	ProductionReader(const char* file);

public:
	typedef std::vector<std::string> ProducitonContainer;

public:
	const ProducitonContainer& GetProductions() const;

private:
	void ReadProductions();

private:
	FileReader fileReader_;
	ProducitonContainer grammars_;
};
