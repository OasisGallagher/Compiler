#include <cassert>
#include <algorithm>
#include "debug.h"
#include "scanner.h"
#include "constants.h"
#include "utilities.h"
#include "file_reader.h"

enum ScannerStateType {
	ScannerStateStart,
	ScannerStateAssign,

	ScannerStateLess,
	ScannerStateGreater,

	ScannerStateDoubleQuotes,

	ScannerStateID,
	ScannerStateNumber,
	ScannerStateDone
};

static DummyToken dummyToken;

LineScanner::LineScanner() 
	: current_(nullptr), dest_(nullptr) {
	lineBuffer_ = new char[Constants::kMaxLineCharacters]();
	tokenBuffer_ = new char[Constants::kMaxTokenCharacters]();
}

LineScanner::~LineScanner() {
	delete[] lineBuffer_;
	delete[] tokenBuffer_;
}

void LineScanner::SetText(const char* text) {
	size_t length = strlen(text);
	Assert(length > 0 && length < Constants::kMaxLineCharacters, "invalid line text");

	std::copy(text, text + length, lineBuffer_);
	lineBuffer_[length] = 0;

	current_ = lineBuffer_;
	dest_ = lineBuffer_ + length + 1;
}

bool LineScanner::GetChar(int* ch) {
	if (current_ == nullptr || current_ == dest_) {
		return false;
	}

	*ch = *current_++;
	return true;
}

void LineScanner::UngetChar() {
	Assert(current_ != nullptr && current_ > lineBuffer_, "unget failed. invalid state");
	--current_;
}

ScannerTokenType LineScanner::GetToken(char* token) {
	ScannerStateType state = ScannerStateStart;
	ScannerTokenType tokenType = ScannerTokenError;

	bool savech = false, unget = false;
	int ch = 0;

	int index = 0;

	for (; state != ScannerStateDone;) {
		if (!GetChar(&ch)) {
			return ScannerTokenEndOfFile;
		}

		savech = true;
		unget = false;

		switch (state) {
		case ScannerStateStart:
			if (ch == '#') {
				return ScannerTokenEndOfFile;
			}

			if (ch == ' ' || ch == '\t' || ch == '\n' || ch == 0) {
				savech = false;
			}
			else if (isdigit(ch)) {
				state = ScannerStateNumber;
			}
			else if (isalpha(ch) || ch == '_' || ch == '$') {
				state = ScannerStateID;
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
			else if (ch == '"') {
				state = ScannerStateDoubleQuotes;
				savech = false;
			}
			else {
				state = ScannerStateDone;

				switch (ch) {
				case '+':
					tokenType = ScannerTokenPlus;
					break;

				case '-':
					tokenType = ScannerTokenMinus;
					break;

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

				case ';':
					tokenType = ScannerTokenSemicolon;
					break;

				case ':':
					tokenType = ScannerTokenColon;
					break;
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
			}
			break;

		case ScannerStateDoubleQuotes:
			if (ch == '"') {
				tokenType = ScannerTokenString;
				savech = false;
				state = ScannerStateDone;
			}
			break;

		case ScannerStateID:
			if (!isdigit(ch) && !isalpha(ch) && ch != '_' && ch != '$') {
				state = ScannerStateDone;
				tokenType = ScannerTokenID;
				unget = true;
				savech = false;
			}
			break;

		case ScannerStateNumber:
			if (!isdigit(ch)) {
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
			Assert(index < Constants::kMaxTokenCharacters, "invalid token. buffer too small.");
			tokenBuffer_[index++] = ch;
		}
	}
	
	std::copy(tokenBuffer_, tokenBuffer_ + index, token);
	token[index] = 0;

	return tokenType;
}

FileScanner::FileScanner(const char* path) 
	: reader_(new FileReader(path)) {
	symbols_ = new Table < Symbol >();
	numberLiterals_ = new Table < NumberLiteral >();
	stringLiterals_ = new Table < StringLiteral >();
}

FileScanner::~FileScanner() {
	delete reader_;
	delete symbols_;
	delete numberLiterals_;
	delete stringLiterals_;
}

bool FileScanner::GetToken(ScannerToken* token) {
	char buffer[Constants::kMaxTokenCharacters];
	ScannerTokenType tokenType = lineScanner_.GetToken(buffer);

	char line[Constants::kMaxLineCharacters];
	for (; tokenType == ScannerTokenEndOfFile; ) {
		if (!reader_->ReadLine(line, Constants::kMaxLineCharacters)) {
			return false;
		}

		if (strlen(line) == 0) {
			continue;
		}

		lineScanner_.SetText(line);
		tokenType = lineScanner_.GetToken(buffer);
	}

	if(tokenType == ScannerTokenError) {
		return false;
	}

	token->tokenType = tokenType;
	token->token = &dummyToken;

	if (tokenType == ScannerTokenNumber) {
		token->token = numberLiterals_->Add(buffer);
	}
	else if (tokenType == ScannerTokenString) {
		token->token = stringLiterals_->Add(buffer);
	}
	else if (tokenType == ScannerTokenID) {
		ScannerTokenType reserveTokenType = GetReserveTokenType(buffer);
		if (reserveTokenType != ScannerTokenError) {
			token->tokenType = reserveTokenType;
		}
		else {
			token->token = symbols_->Add(buffer);
		}
	}

	return true;
}

ScannerTokenType FileScanner::GetReserveTokenType(const char* name) {
	if (strcmp(name, "if") == 0) {
		return ScannerTokenIf;
	}

	if (strcmp(name, "else") == 0) {
		return ScannerTokenElse;
	}

	if (strcmp(name, "then") == 0) {
		return ScannerTokenThen;
	}

	if (strcmp(name, "end") == 0) {
		return ScannerTokenEnd;
	}

	return ScannerTokenError;
}
