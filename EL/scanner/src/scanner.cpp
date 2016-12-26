#include <cassert>
#include <sstream>
#include <algorithm>
#include "debug.h"
#include "define.h"
#include "scanner.h"
#include "utilities.h"
#include "reader.h"

struct Compound {
	const char* text;
	ScannerTokenType type;

	bool operator < (const Compound& other) const {
		return strcmp(text, other.text) < 0;
	}
};

static Compound compounds[] = {
	"+", ScannerTokenPlus,
	"-", ScannerTokenMinus,
	"*", ScannerTokenMultiply,
	"/", ScannerTokenDivide,
	"|", ScannerTokenBitwiseOr,
	"(", ScannerTokenLeftBrace,
	")", ScannerTokenRightBrace,
	"{", ScannerTokenLeftParenthesis,
	"}", ScannerTokenRightParenthesis,
	";", ScannerTokenSemicolon,
	":", 
	"<", ScannerTokenLess,
	"<=", ScannerTokenLessEqual,
};

enum ScannerStateType {
	ScannerStateStart = -1,
	ScannerStateDone,

	ScannerStateNumber,
	ScannerStateString,
	ScannerStateIdentifier,
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
}

TextScanner::~TextScanner() {
	delete[] lineBuffer_;
	delete[] tokenBuffer_;
}

void TextScanner::SetText(const char* text) {
	size_t length = strlen(text);
	Assert(length > 0 && length < MAX_LINE_CHARACTERS, "invalid line text");

	std::copy(text, text + length, lineBuffer_);
	lineBuffer_[length] = 0;

	start_ = current_ = lineBuffer_;
	dest_ = lineBuffer_ + length + 1;
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
	return GetNextToken(token, pos);
}

ScannerTokenType TextScanner::GetNextToken(char* token, int* pos) {
	int count = sizeof(compounds) / sizeof(compounds[0]);
	std::sort(compounds, compounds + count);
	int currentState = -1;

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
			else if (ch == '"') {
				state = ScannerStateString;
				savech = false;
			}
			else if (ch == '\n') {
				state = ScannerStateDone;
				tokenType = ScannerTokenNewline;
			}
			else {
				int low = 0, high = 0;
				for (; low < count && *compounds[low].text != ch; low++) {
				}

				Assert(low < count, "can not find item leading with " + std::to_string(ch));
				for (high = low; high < count && *compounds[high].text == ch; ++high) {
				}

				if (high - low == 1 && strlen(compounds[low].text) == 1) {
					currentState = 0;
					tokenType = compounds[low].type;
				}
				else {
					currentState = Utility::MakeDword(low, high);
				}
			}
			break;

		case ScannerStateString:
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

		default:
			int low = Utility::Loword(currentState);
			int high = Utility::Highword(currentState);
			Assert(high != 0, "invalid state " + std::to_string(currentState));
			for (; low != high; ++low) {
				if (compounds[low].text[1] == ch) {
					currentState = 0;
					tokenType = compounds[low].type;
					break;
				}
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
