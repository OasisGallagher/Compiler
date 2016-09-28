#pragma once
#include <string>

class Parser;
class SyntaxTree;

struct LanguageParameter {
	const char* productions;
};

class Language {
public:
	Language(LanguageParameter* parameter);
	~Language();

public:
	bool Parse(/*SyntaxTree* tree, */const std::string& file);
	std::string ToString() const;

private:
	Parser* parser_;
};
