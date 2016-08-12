#pragma once

#include <fstream>
#include <map>
#include <string>
#include "types.h"

// Scanner扫描到的标记.
enum ScannerTokenType {
	// 错误.
	ScannerTokenError,
	// 文件结束.
	ScannerTokenEndOfFile,

	// 保留字if.
	ScannerTokenIf,
	// 保留字then.
	ScannerTokenThen,
	// 保留字else.
	ScannerTokenElse,
	// 保留字end.
	ScannerTokenEnd,

	// ID(this, 符号表指针).
	ScannerTokenID,
	// 数字(this, 常数表指针).
	ScannerTokenNumber,

	// 赋值.
	ScannerTokenAssign,
	// 相等.
	ScannerTokenEqual,
	// 小于.
	ScannerTokenLess,
	// 小于等于.
	ScannerTokenLessEqual,
	// 大于.
	ScannerTokenGreater,
	// 大于等于.
	ScannerTokenGreaterEqual,
	// 加.
	ScannerTokenPlus,
	// 减.
	ScannerTokenMinus,
	// 乘.
	ScannerTokenMultiply,
	// 除.
	ScannerTokenDivide,

	// 位或, |.
	ScannerTokenXor,

	// 字符串.
	ScannerTokenString,

	// 左括号.
	ScannerTokenLeftParenthesis,
	// 右括号.
	ScannerTokenRightParenthesis,
	// 分号.
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

