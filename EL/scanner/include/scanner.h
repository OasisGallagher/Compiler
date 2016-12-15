#pragma once

#include <map>
#include <string>
#include "token.h"

class TextScanner {
public:
	TextScanner();
	~TextScanner();

	void SetText(const char* text);
	ScannerTokenType GetToken(char* token, int* pos = nullptr);

private:
	bool GetChar(int* ch);
	void UngetChar();
	ScannerTokenType GetNextToken(char* token, int* pos = nullptr);

	bool IsUnaryOperator();

	char* lineBuffer_;
	char* tokenBuffer_;

	char* start_;
	char* dest_;
	char* current_;

	char* lastToken_;
	ScannerTokenType lastTokenType_;
};

class FileReader;

struct TokenPosition {
	int lineno;
	int linepos;

	std::string ToString() const;
};

class FileScanner {
public:
	FileScanner(const char* path);
	~FileScanner();

public:
	bool GetToken(ScannerToken* token, TokenPosition* pos);

private:
	int lineno_;
	FileReader* reader_;
	TextScanner textScanner_;
};
