#pragma once
#include <string>

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

	// �ָ���, |.
	ScannerTokenSeperator,

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

struct ScannerToken {
	ScannerTokenType tokenType;
	Token* token;
};
