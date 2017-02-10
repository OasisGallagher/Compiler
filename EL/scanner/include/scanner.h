#pragma once

#include <string>
#include "token_define.h"

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

private:
	char* lineBuffer_;
	char* tokenBuffer_;

	char* start_;
	char* dest_;
	char* current_;
};

class FileReader;

struct TokenPosition {
	int lineno;
	int linepos;

	std::string ToString() const;
};

class FileScanner {
public:
	FileScanner(const char* fileName);
	~FileScanner();

public:
	bool GetToken(ScannerToken* token, TokenPosition* pos);

private:
	int lineno_;
	FileReader* reader_;
	TextScanner textScanner_;
};
