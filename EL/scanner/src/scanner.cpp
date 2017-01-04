#include <cassert>
#include <sstream>
#include <algorithm>
#include "debug.h"
#include "define.h"
#include "scanner.h"
#include "utilities.h"
#include "reader.h"

#define STATE_START			-1
#define STATE_DONE			0
#define STATE_NUMBER		1
#define STATE_STRING		2
#define STATE_IDENTIFIER	3

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

void Narrow(int& low, int& high, int count, int ci, int ch) {
	for (; low < count && compounds[low].text[ci] != ch; ++low) {
	}

	for (high = low + 1; high < count && compounds[high].text[ci] == ch; ++high) {
	}
}

ScannerTokenType TextScanner::GetNextToken(char* token, int* pos) {
	int count = sizeof(compounds) / sizeof(compounds[0]);
	std::sort(compounds, compounds + count);

	int ci = 0, bi = 0, ch = 0, state = STATE_START;
	bool savech = true, unget = false;

	ScannerTokenType tokenType = ScannerTokenError;

	for (; state != STATE_DONE;) {
		if (!GetChar(&ch)) {
			if (pos != nullptr) {
				*pos = -1;
			}

			return ScannerTokenEndOfFile;
		}

		savech = true;
		unget = false;

		switch (state) {
		case STATE_START:
			if (ch == '#') {
				return ScannerTokenEndOfFile;
			}

			if (ch == ' ' || ch == '\t' || ch == 0) {
				savech = false;
			}
			else if (Utility::IsDigit(ch)) {
				state = STATE_NUMBER;
			}
			else if (Utility::IsLetter(ch)) {
				state = STATE_IDENTIFIER;
			}
			else if (ch == '"') {
				state = STATE_STRING;
				savech = false;
			}
			else if (ch == '\n') {
				state = STATE_DONE;
				tokenType = ScannerTokenNewline;
			}
			else {
				int low = 0, high = 0;

				for (; low < count && compounds[low].text[ci] != ch; ++low) {
				}

				for (high = low + 1; high < count && compounds[high].text[ci] == ch; ++high) {
				}

				Assert(high <= count, std::string("can not find item leading with ") + (char)ch + ".");

				++ci;
				if (high - low == 1 && strlen(compounds[low].text) == ci) {
					state = STATE_DONE;
					tokenType = compounds[low].type;
				}
				else {
					state = Utility::MakeDword(low, high);
				}

				savech = true;
			}
			break;

		case STATE_STRING:
			if (ch == '"') {
				tokenType = ScannerTokenString;
				savech = false;
				state = STATE_DONE;
			}
			break;

		case STATE_IDENTIFIER:
			if (!Utility::IsDigit(ch) && !Utility::IsLetter(ch)) {
				state = STATE_DONE;
				tokenType = ScannerTokenIdentifier;
				unget = true;
				savech = false;
			}
			break;

		case STATE_NUMBER:
			if (!Utility::IsDigit(ch)) {
				state = STATE_DONE;
				tokenType = ScannerTokenNumber;
				unget = true;
				savech = false;
			}
			break;

		default:
			int low = Utility::Loword(state), high = Utility::Highword(state);
			
			savech = false;
			if (ch == 0) {
				Assert(strlen(compounds[low].text) == ci, "invalid symbol");
				state = STATE_DONE;
				tokenType = compounds[low].type;
			}
			else {
				int nl = low;
				for (; nl < high && compounds[nl].text[ci] != ch; ++nl) {
				}

				if (nl < high) {
					low = nl;
				}

				int nh = low + 1;
				for (; nh < high && compounds[nh].text[ci] == ch; ++nh) {
				}
				high = nh;

				savech = (compounds[low].text[ci] == ch);
				unget = !savech;

				if (savech) {
					++ci;
				}

				if (high - low == 1 && strlen(compounds[low].text) == ci) {
					state = STATE_DONE;
					tokenType = compounds[low].type;
				}
				else {
					state = Utility::MakeDword(low, high);
				}
			}

			break;
		}

		if (unget) {
			UngetChar();
		}

		if (savech) {
			Assert(bi < MAX_TOKEN_CHARACTERS, "invalid token.");
			tokenBuffer_[bi++] = ch;
		}
	}

	strncpy(token, tokenBuffer_, bi);
	token[bi] = 0;

	if (pos != nullptr) {
		*pos = 1 + current_ - start_ - bi;
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
