#pragma once
#include "define.h"

enum ScannerTokenType {
	ScannerTokenError,
	ScannerTokenEndOfFile,

	ScannerTokenIdentifier,
	ScannerTokenNumber,
	ScannerTokenString,

	ScannerTokenNewline,

	ScannerTokenPlus,
	ScannerTokenMinus,
	ScannerTokenMultiply,
	ScannerTokenDivide,
	ScannerTokenMod,
	
	ScannerTokenPlusEqual,
	ScannerTokenMinusEqual,
	ScannerTokenMultiplyEqual,
	ScannerTokenDivideEqual,
	ScannerTokenModEqual,

	ScannerTokenBitwiseAnd,
	ScannerTokenBitwiseOr,
	ScannerTokenBitwiseXor,

	ScannerTokenBitwiseAndEqual,
	ScannerTokenBitwiseOrEqual,
	ScannerTokenBitwiseXorEqual,

	ScannerTokenLess,
	ScannerTokenGreater,
	ScannerTokenAssign,

	ScannerTokenLessEqual,
	ScannerTokenGreaterEqual,
	ScannerTokenEqual,

	ScannerTokenLeftBrace,
	ScannerTokenRightBrace,

	ScannerTokenLeftParenthesis,
	ScannerTokenRightParenthesis,
	
	ScannerTokenLeftSquareBracket,
	ScannerTokenRightSquareBracket,
	
	ScannerTokenQuestionmark,
	ScannerTokenSemicolon,
	ScannerTokenColon,
	ScannerTokenComma,
	ScannerTokenDot,
};

struct ScannerToken {
	ScannerTokenType tokenType;
	char text[MAX_TOKEN_CHARACTERS];
};
