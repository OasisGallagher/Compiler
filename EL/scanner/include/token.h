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

	// ����.
	ScannerTokenPositive,
	// ����.
	ScannerTokenNegative,
	
	// �ָ���, |.
	ScannerTokenSeperator,

	// �ַ���.
	ScannerTokenString,

	// ���з�.
	ScannerTokenNewline,

	// ������.
	ScannerTokenLeftBrace,
	// �һ�����.
	ScannerTokenRightBrace,
	
	// ��Բ����.
	ScannerTokenLeftParenthesis,
	// ��Բ����.
	ScannerTokenRightParenthesis,

	// �ֺ�.
	ScannerTokenSemicolon,

	// ð��.
	ScannerTokenColon,

	// ����.
	ScannerTokenComma,
};

struct ScannerToken {
	ScannerTokenType tokenType;
	char text[MAX_TOKEN_CHARACTERS];
};
