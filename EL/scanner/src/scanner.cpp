#include <cassert>
#include <sstream>
#include <algorithm>
#include "debug.h"
#include "define.h"
#include "scanner.h"
#include "utilities.h"
#include "reader.h"

enum ScannerStateType {
	ScannerStateStart,
	ScannerStateAssign,

	ScannerStateLess,
	ScannerStateGreater,

	ScannerStatePlus,
	ScannerStateMinus,
	ScannerStateDoubleQuotes,

	ScannerStateIdentifier,
	ScannerStateNumber,
	ScannerStateDone
};

std::string TokenPosition::ToString() const {
	std::ostringstream oss;
	oss << lineno << ":" << linepos;
	return oss.str();
}

TextScanner::TextScanner() 
	: current_(nullptr), dest_(nullptr) {
	lineBuffer_ = new char[MAX_LINE_CHARACTERS];
	std::fill(lineBuffer_, lineBuffer_ + MAX_LINE_CHARACTERS, 0);

	tokenBuffer_ = new char[MAX_TOKEN_CHARACTERS];
	std::fill(tokenBuffer_, tokenBuffer_ + MAX_TOKEN_CHARACTERS, 0);

	lastToken_ = new char[MAX_TOKEN_CHARACTERS];
	std::fill(lastToken_, lastToken_ + MAX_TOKEN_CHARACTERS, 0);

	lastTokenType_ = ScannerTokenEndOfFile;
}

TextScanner::~TextScanner() {
	delete[] lineBuffer_;
	delete[] tokenBuffer_;
	delete[] lastToken_;
}

void TextScanner::SetText(const char* text) {
	size_t length = strlen(text);
	Assert(length > 0 && length < MAX_LINE_CHARACTERS, "invalid line text");

	std::copy(text, text + length, lineBuffer_);
	lineBuffer_[length] = 0;

	start_ = current_ = lineBuffer_;
	dest_ = lineBuffer_ + length + 1;

	lastTokenType_ = ScannerTokenEndOfFile;
	std::fill(lastToken_, lastToken_ + MAX_TOKEN_CHARACTERS, 0);
}

bool TextScanner::GetChar(int* ch) {
	if (current_ == nullptr || current_ == dest_) {
		return false;
	}

	*ch = *current_++;
	return true;
}

void TextScanner::UngetChar() {
	Assert(current_ != nullptr && current_ > lineBuffer_, "unget failed. invalid state");
	--current_;
}

ScannerTokenType TextScanner::GetToken(char* token, int* pos) {
	*token = 0;
	lastTokenType_ = GetNextToken(token, pos);
	strcpy(lastToken_, token);
	return lastTokenType_;
}

ScannerTokenType TextScanner::GetNextToken(char* token, int* pos) {
	ScannerStateType state = ScannerStateStart;
	ScannerTokenType tokenType = ScannerTokenError;

	bool savech = false, unget = false;
	int ch = 0;

	int index = 0;

	for (; state != ScannerStateDone;) {
		if (!GetChar(&ch)) {
			if (pos != nullptr) {
				*pos = -1;
			}

			return ScannerTokenEndOfFile;
		}

		savech = true;
		unget = false;

		switch (state) {
		case ScannerStateStart:
			if (ch == '#') {
				return ScannerTokenEndOfFile;
			}

			if (ch == ' ' || ch == '\t' || ch == 0) {
				savech = false;
			}
			else if (Utility::IsDigit(ch)) {
				state = ScannerStateNumber;
			}
			else if (Utility::IsLetter(ch)) {
				state = ScannerStateIdentifier;
			}
			else if (ch == '<') {
				state = ScannerStateLess;
			}
			else if (ch == '=') {
				state = ScannerStateAssign;
			}
			else if (ch == '>') {
				state = ScannerStateGreater;
			}
			else if (ch == '+') {
				state = ScannerStatePlus;
			}
			else if (ch == '-') {
				state = ScannerStateMinus;
			}
			else if (ch == '"') {
				state = ScannerStateDoubleQuotes;
				savech = false;
			}
			else if (ch == '\n') {
				state = ScannerStateDone;
				tokenType = ScannerTokenNewline;
			}
			else {
				state = ScannerStateDone;

				switch (ch) {
				case '*':
					tokenType = ScannerTokenMultiply;
					break;

				case '/':
					tokenType = ScannerTokenDivide;
					break;

				case '|':
					tokenType = ScannerTokenSeperator;
					break;

				case '(':
					tokenType = ScannerTokenLeftParenthesis;
					break;

				case ')':
					tokenType = ScannerTokenRightParenthesis;
					break;

				case '{':
					tokenType = ScannerTokenLeftBrace;
					break;;

				case '}':
					tokenType = ScannerTokenRightBrace;
					break;

				case ';':
					tokenType = ScannerTokenSemicolon;
					break;

				case ':':
					tokenType = ScannerTokenColon;
					break;

				case ',':
					tokenType = ScannerTokenComma;
					break;

				default:
					Assert(false, std::string("unrecognized sign: ") + (char)ch);
					tokenType = ScannerTokenError;
				}
			}
			break;

		case ScannerStateAssign:
			state = ScannerStateDone;

			if (ch == '=') {
				tokenType = ScannerTokenEqual;
			}
			else {
				tokenType = ScannerTokenAssign;
				unget = true;
				savech = false;
			}
			break;

		case ScannerStateMinus:
			unget = true;
			savech = false;
			state = ScannerStateDone;

			if (IsUnaryOperator()) {
				strcpy(tokenBuffer_, NEGATIVE_SIGN);
				index = strlen(NEGATIVE_SIGN);
				tokenType = ScannerTokenNegative;
			}
			else {
				tokenType = ScannerTokenMinus;
			}
			break;


		case ScannerStatePlus:
			savech = false;
			unget = true;
			state = ScannerStateDone;

			if (IsUnaryOperator()) {
				strcpy(tokenBuffer_, POSITIVE_SIGN);
				index = strlen(POSITIVE_SIGN);
				tokenType = ScannerTokenPositive;
			}
			else {
				tokenType = ScannerTokenPlus;
			}
			break;

		case ScannerStateLess:
			state = ScannerStateDone;
			if (ch == '=') {
				tokenType = ScannerTokenLessEqual;
			}
			else {
				tokenType = ScannerTokenLess;
				unget = true;
				savech = false;
			}
			break;

		case ScannerStateGreater:
			state = ScannerStateDone;
			if (ch == '=') {
				tokenType = ScannerTokenGreaterEqual;
			}
			else {
				tokenType = ScannerTokenGreater;
				unget = true;
				savech = false;
			}
			break;

		case ScannerStateDoubleQuotes:
			if (ch == '"') {
				tokenType = ScannerTokenString;
				savech = false;
				state = ScannerStateDone;
			}
			break;

		case ScannerStateIdentifier:
			if (!Utility::IsDigit(ch) && !Utility::IsLetter(ch)) {
				state = ScannerStateDone;
				tokenType = ScannerTokenIdentifier;
				unget = true;
				savech = false;
			}
			break;

		case ScannerStateNumber:
			if (!Utility::IsDigit(ch)) {
				state = ScannerStateDone;
				tokenType = ScannerTokenNumber;
				unget = true;
				savech = false;
			}
			break;
		}

		if (unget) {
			UngetChar();
		}

		if (savech) {
			Assert(index < MAX_TOKEN_CHARACTERS, "invalid token.");
			tokenBuffer_[index++] = ch;
		}
	}

	strncpy(token, tokenBuffer_, index);
	token[index] = 0;

	if (pos != nullptr) {
		*pos = 1 + current_ - start_ - index;
	}

	return tokenType;
}

bool TextScanner::IsUnaryOperator() {
	switch (lastTokenType_) {
	case ScannerTokenLess:
	case ScannerTokenPlus:
	case ScannerTokenMinus:
	case ScannerTokenComma:
	case ScannerTokenEqual:
	case ScannerTokenDivide:
	case ScannerTokenAssign:
	case ScannerTokenNewline:
	case ScannerTokenGreater:
	case ScannerTokenMultiply:
	case ScannerTokenNegative:
	case ScannerTokenPositive:
	case ScannerTokenLessEqual:
	case ScannerTokenSemicolon:
	case ScannerTokenEndOfFile:
	case ScannerTokenGreaterEqual:
	case ScannerTokenLeftParenthesis:
		return true;
	}

	return false;
}

FileScanner::FileScanner(const char* path) 
	: reader_(new FileReader(path, true, false)), lineno_(0) {
}

FileScanner::~FileScanner() {
	delete reader_;
}

bool FileScanner::GetToken(ScannerToken* token, TokenPosition* pos) {
	char buffer[MAX_TOKEN_CHARACTERS] = { 0 };
	ScannerTokenType tokenType = textScanner_.GetToken(buffer, &pos->linepos);

	char line[MAX_LINE_CHARACTERS];
	for (; tokenType == ScannerTokenEndOfFile; ) {
		if (!reader_->ReadLine(line, MAX_LINE_CHARACTERS, &lineno_)) {
			tokenType = ScannerTokenEndOfFile;
			break;
		}

		if (Utility::IsBlankText(line)) {
			continue;
		}

		textScanner_.SetText(line);
		tokenType = textScanner_.GetToken(buffer, &pos->linepos);
	}

	pos->lineno = lineno_;

	if(tokenType == ScannerTokenError) {
		return false;
	}

	token->tokenType = tokenType;
	strcpy(token->text, buffer);

	return true;
}
