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

	// 字符串.
	ScannerTokenString,

	// \n
	ScannerTokenNewline,

	// {
	ScannerTokenLeftBrace,
	// }
	ScannerTokenRightBrace,

	// (
	ScannerTokenLeftParenthesis,
	// )
	ScannerTokenRightParenthesis,

	// ;
	ScannerTokenSemicolon,

	// :
	ScannerTokenColon,

	// .
	ScannerTokenComma,

	// ?
	ScannerTokenQuestionmark,

	// =
	ScannerTokenAssign,
	// ==
	ScannerTokenEqual,

	// <
	ScannerTokenLess,
	// <=
	ScannerTokenLessEqual,

	// >
	ScannerTokenGreater,
	// >=
	ScannerTokenGreaterEqual,

	// +
	ScannerTokenPlus,
	// +=
	ScannerTokenPlusEqual,

	// -
	ScannerTokenMinus,
	// -=
	ScannerTokenMinusEqual,

	// *
	ScannerTokenMultiply,
	// *=
	ScannerTokenMultiplyEqual,

	// /
	ScannerTokenDivide,
	// /=
	ScannerTokenDivideEqual,
	
	// |
	ScannerTokenBitwiseOr,
	// |=
	ScannerTokenBitwiseOrEqual,

	// %
	ScannerTokenMod,
	// %=
	ScannerTokenModEqual,

	// &
	ScannerTokenBitwiseAnd,
	// &=
	ScannerTokenBitwiseAndEqual,

	// ^
	ScannerTokenBitwiseXor,
	// ^=
	ScannerTokenBitwiseXorEqual,
};

struct ScannerToken {
	ScannerTokenType tokenType;
	char text[MAX_TOKEN_CHARACTERS];
};
