#include <cstring>
#include <algorithm>

#include "tokens.h"

Tokens Tokens::instance;

struct TokenDefine {
	const char* text;
	ScannerTokenType type;

	bool operator < (const TokenDefine& other) const {
		return strcmp(text, other.text) < 0;
	}
};

static TokenDefine tokens[] = {
	"//", ScannerTokenEndOfFile,

	"+", ScannerTokenPlus,
	"-", ScannerTokenMinus,
	"*", ScannerTokenMultiply,
	"/", ScannerTokenDivide,
	"%", ScannerTokenMod,

	"+=", ScannerTokenPlusEqual,
	"-=", ScannerTokenMinusEqual,
	"*=", ScannerTokenMultiplyEqual,
	"/=", ScannerTokenDivideEqual,
	"%=", ScannerTokenModEqual,

	"++", ScannerTokenSelfIncrement,
	"--", ScannerTokenSelfDecrement,

	"<<", ScannerTokenShiftLeft,
	">>", ScannerTokenShiftRight,

	"<<=", ScannerTokenShiftLeftEqual,
	">>=", ScannerTokenShiftRightEqual,

	"&", ScannerTokenBitwiseAnd,
	"|", ScannerTokenBitwiseOr,
	"^", ScannerTokenBitwiseXor,
	"~", ScannerTokenBitwiseNot,

	"&=", ScannerTokenBitwiseAndEqual,
	"|=", ScannerTokenBitwiseOrEqual,
	"^=", ScannerTokenBitwiseXorEqual,

	"<", ScannerTokenLess,
	">", ScannerTokenGreater,
	"=", ScannerTokenAssign,

	"||", ScannerTokenOr,
	"&&", ScannerTokenAnd,

	"<=", ScannerTokenLessEqual,
	">=", ScannerTokenGreaterEqual,
	"==", ScannerTokenEqual,
	"!=", ScannerTokenNotEqual,

	"{", ScannerTokenLeftBrace,
	"}", ScannerTokenRightBrace,

	"(", ScannerTokenLeftParenthesis,
	")", ScannerTokenRightParenthesis,

	"[", ScannerTokenLeftSquareBracket,
	"]", ScannerTokenRightSquareBracket,

	"?", ScannerTokenQuestionmark,
	"!", ScannerTokenExclamation,
	";", ScannerTokenSemicolon,
	":", ScannerTokenColon,
	",", ScannerTokenComma,
	".", ScannerTokenDot,
};

Tokens::Tokens() {
	int count = sizeof(tokens) / sizeof(tokens[0]);
	std::sort(tokens, tokens + count);
}

int Tokens::Size() {
	return sizeof(tokens) / sizeof(tokens[0]);
}

ScannerTokenType Tokens::Type(int index) {
	return tokens[index].type;
}

const char* Tokens::Text(int index) {
	return tokens[index].text;
}
