#pragma once
#include <string>
#include "constants.h"

// Scannerɨ�赽�ı��.
enum ScannerTokenType {
	// ����.
	ScannerTokenError,
	// �ļ�����.
	ScannerTokenEndOfFile,

	// ID(this, ���ű�ָ��).
	ScannerTokenID,
	// ����(this, ������ָ��).
	ScannerTokenNumber,
	
	// ����.
	ScannerTokenSign,
	/*
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
	*/

	// �ָ���, |.
	//ScannerTokenSeperator,

	// �ַ���.
	ScannerTokenString,
	/*
	// ������.
	ScannerTokenLeftParenthesis,
	// ������.
	ScannerTokenRightParenthesis,
	// �ֺ�.
	ScannerTokenSemicolon,
	// ð��.
	ScannerTokenColon,*/
};

// class Token {
// public:
// 	Token(const std::string& text) : text_(text) {
// 
// 	}
// 
// public:
// 	virtual std::string ToString() = 0;
// 
// 	const std::string& Text() const {
// 		return text_;
// 	}
// 
// private:
// 	std::string text_;
// };
// 
// class DummyToken : public Token {
// public:
// 	DummyToken() : Token("") {
// 
// 	}
// 
// 	virtual std::string ToString() {
// 		return "null";
// 	}
// };
// 
// class Symbol : public Token {
// public:
// 	Symbol(const char* text) :Token(text) {
// 		name_ = text;
// 	}
// 
// 	virtual std::string ToString() {
// 		return name_;
// 	}
// 
// private:
// 	std::string name_;
// };
// 
// class Literal : public Token {
// public:
// 	Literal(const char* text) : Token(text) {
// 
// 	}
// };
// 
// class NumberLiteral : public Literal {
// public:
// 	NumberLiteral(const char* text) : Literal(text) {
// 		value_ = atoi(text);
// 	}
// 
// 	virtual std::string ToString() {
// 		return std::to_string(value_);
// 	}
// private:
// 	int value_;
// };
// 
// class StringLiteral : public Literal {
// public:
// 	StringLiteral(const char* text) :Literal(text) {
// 		value_ = text;
// 	}
// 
// 	virtual std::string ToString() {
// 		return value_;
// 	}
// private:
// 	std::string value_;
// };

struct ScannerToken {
	ScannerTokenType tokenType;
	char text[Constants::kMaxTokenCharacters];
};
