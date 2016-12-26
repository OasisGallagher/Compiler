#pragma once
#include <string>
#include "define.h"

// Scannerɨ�赽�ı��.
enum ScannerTokenType {
	// ����.
	ScannerTokenError,
	// �ļ�����.
	ScannerTokenEndOfFile,

	// ID(this, ���ű�ָ��).
	ScannerTokenIdentifier,

	// ����(this, ������ָ��).
	ScannerTokenNumber,

	// �ַ���.
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
