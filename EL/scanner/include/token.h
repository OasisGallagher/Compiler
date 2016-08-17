#pragma once
#include <string>
#include "constants.h"

// Scanner扫描到的标记.
enum ScannerTokenType {
	// 错误.
	ScannerTokenError,
	// 文件结束.
	ScannerTokenEndOfFile,

	// ID(this, 符号表指针).
	ScannerTokenID,
	// 数字(this, 常数表指针).
	ScannerTokenNumber,
	
	// 符号.
	ScannerTokenSign,
	/*
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
	*/

	// 分隔符, |.
	//ScannerTokenSeperator,

	// 字符串.
	ScannerTokenString,
	/*
	// 左括号.
	ScannerTokenLeftParenthesis,
	// 右括号.
	ScannerTokenRightParenthesis,
	// 分号.
	ScannerTokenSemicolon,
	// 冒号.
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
