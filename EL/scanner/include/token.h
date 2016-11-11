#pragma once
#include <string>
#include "define.h"

// Scanner扫描到的标记.
enum ScannerTokenType {
	// 错误.
	ScannerTokenError,
	// 文件结束.
	ScannerTokenEndOfFile,

	// ID(this, 符号表指针).
	ScannerTokenIdentifier,
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

	// 正号.
	ScannerTokenPositive,
	// 负号.
	ScannerTokenNegative,
	
	// 分隔符, |.
	ScannerTokenSeperator,

	// 字符串.
	ScannerTokenString,

	// 换行符.
	ScannerTokenNewline,

	// 左花括号.
	ScannerTokenLeftBrace,
	// 右花括号.
	ScannerTokenRightBrace,
	
	// 左圆括号.
	ScannerTokenLeftParenthesis,
	// 右圆括号.
	ScannerTokenRightParenthesis,

	// 分号.
	ScannerTokenSemicolon,

	// 冒号.
	ScannerTokenColon,

	// 逗号.
	ScannerTokenComma,
};

struct ScannerToken {
	ScannerTokenType tokenType;
	char text[MAX_TOKEN_CHARACTERS];
};
