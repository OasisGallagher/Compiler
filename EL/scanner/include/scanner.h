#pragma once

#include <fstream>
#include <map>
#include <string>
#include "types.h"

// Scannerɨ�赽�ı��.
enum ScannerTokenType {
	// ����.
	ScannerTokenError,
	// �ļ�����.
	ScannerTokenEndOfFile,

	// ������if.
	ScannerTokenIf,
	// ������then.
	ScannerTokenThen,
	// ������else.
	ScannerTokenElse,
	// ������end.
	ScannerTokenEnd,

	// ID(this, ���ű�ָ��).
	ScannerTokenID,
	// ����(this, ������ָ��).
	ScannerTokenNumber,

	// ��ֵ.
	ScannerTokenAssign,
	// ���.
	ScannerTokenEqual,
	// С��.
	ScannerTokenLess,
	// С�ڵ���.
	ScannerTokenLessEqual,
	// ����.
	ScannerTokenGreater,
	// ���ڵ���.
	ScannerTokenGreaterEqual,
	// ��.
	ScannerTokenPlus,
	// ��.
	ScannerTokenMinus,
	// ��.
	ScannerTokenMultiply,
	// ��.
	ScannerTokenDivide,

	// λ��, |.
	ScannerTokenXor,

	// �ַ���.
	ScannerTokenString,

	// ������.
	ScannerTokenLeftParenthesis,
	// ������.
	ScannerTokenRightParenthesis,
	// �ֺ�.
	ScannerTokenSemicolon,
};

class Token {
public:
	virtual std::string ToString() = 0;
};

class DummyToken : public Token {
public:
	virtual std::string ToString() {
		return "null";
	}
};

class Symbol : public Token {
public:
	Symbol(const char* text) {
		name_ = text;
	}

	virtual std::string ToString() {
		return name_;
	}
private:
	std::string name_;
};

class Literal : public Token {
};

class NumberLiteral : public Literal {
public:
	NumberLiteral(const char* text) {
		value_ = atoi(text);
	}

	virtual std::string ToString() {
		return std::to_string(value_);
	}
private:
	int value_;
};

class StringLiteral : public Literal {
public:
	StringLiteral(const char* text) {
		value_ = text;
	}

	virtual std::string ToString() {
		return value_;
	}
private:
	std::string value_;
};

class NumberLiteralContainer : public Table<NumberLiteral> {
};

class StringLiteralContainer : public Table<StringLiteral> {
};

class SymbolContainer : public Table<Symbol> {
};

class FileReader {
public:
	FileReader(const char* path);
	~FileReader();

public:
	bool ReadLine(char* buffer, size_t length);

private:
	std::ifstream ifs_;
};

struct ScannerToken {
	ScannerTokenType tokenType;
	Token* token;
};

enum ScannerStateType {
	ScannerStateStart,
	ScannerStateAssign,

	ScannerStateLess,
	ScannerStateGreater,

	ScannerStateDoubleQuotes,

	ScannerStateID,
	ScannerStateNumber,
	ScannerStateDone
};

class LineScanner {
public:
	LineScanner();
	~LineScanner();

	void SetText(const char* text);
	ScannerTokenType GetToken(char* token);

private:
	bool GetChar(int* ch);
	void UngetChar();

	int tokenBufferIndex_;
	char* lineBuffer_;
	char* tokenBuffer_;
	char* dest_;
	char* current_;
};

class Scanner {
public:
	Scanner(const char* path);
	~Scanner();

	bool GetToken(ScannerToken* token);

private:
	ScannerTokenType GetReserveTokenType(const char* name);
private:
	FileReader reader_;
	LineScanner lineScanner_;

	SymbolContainer symbols_;
	NumberLiteralContainer numberLiterals_;
	StringLiteralContainer stringLiterals_;
};

