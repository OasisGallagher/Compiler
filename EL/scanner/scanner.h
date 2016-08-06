// Scanner扫描到的标记.
enum ScannerTokenType {
	// 错误.
	ScannerTokenError,
	// 文件结束.
	ScannerTokenEndOfFile,

	// 保留字if.
	ScannerTokenIf,
	// 保留字then.
	ScannerTokenThen,
	// 保留字else.
	ScannerTokenElse,
	// 保留字end.
	ScannerTokenEnd,
	// 保留字repeat.
	ScannerTokenRepeat,
	// 保留字until.
	ScannerTokenUntil,
	// 保留字read.
	ScannerTokenRead,
	// 保留字write.
	ScannerTokenWrite,

	// ID(this, 符号表指针).
	ScannerTokenID,
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

	// 左括号.
	ScannerTokenLeftParenthesis,
	// 右括号.
	ScannerTokenRightParenthesis,
	// 分号.
	ScannerTokenSemicolon,
};

class FileReader {
public:
	FileReader(const char* path);

public:
	bool GetNextChar(int* ch);
	void UngetNextChar();

private:
	bool ReadBuffer();

private:
	int lineNumber;
	int linePosition;

	std::ifstream* ifs_;

	char* current_;
	char* buffer_;
	const int kBufferSize = 256;
};

FileReader::FileReader(const char* path) {
	ifs_ = new std::ifstream();
	ifs_.open(path);

	buffer_ = new char[kBufferSize];
}

FileReader::~FileReader() {
	ifs_.close();
	delete ifs_;

	delete[] buffer_;
}

bool FileReader::GetNextChar(int* ch) {
	if (current_ == NULL || *current_ == 0) {
		if (!ReadBuffer()) {
			return false;
		}

		current_ = buffer_;
		++lineNumber;
		linePosition = 0;
	}

	*ch = *current_++;
	++linePosition;
	return true;
}

void FileReader::UngetNextChar() {
	if (linePosition != 0) {
		--linePosition;
	}
}

bool FileReader::ReadBuffer() {
	return ifs_.getline(buffer_, kBufferSize);
}

struct ScannerToken {
	ScannerTokenType tokenType;
	void* token;
};

enum ScannerStateType {
	ScannerStateStart,
	ScannerStateAssign,

	ScannerStateLess,
	ScannerStateGreater,

	ScannerStateSingleQuotes,
	ScannerStateDoubleQuotes,

	ScannerStateComment,

	ScannerStateID,
	ScannerStateNumber,
	ScannerStateDone
};

class Scanner {
public:
	Scanner(const char* path);
	~Scanner();

	bool GetToken(ScannerToken* token);

private:
	ScannerStateType CheckCharInStartState(int ch, ScannerToken* token);

private:
	FileReader* reader_;
};

Scanner::Scanner(const char* path) {
	reader_ = new Scanner(path);
}

Scanner::~Scanner() {
	delete reader_;
}

bool Scanner::GetToken(ScannerToken* token) {
	ScannerStateType state = ScannerStateStart;
	ScannerTokenType tokenType = ScannerTokenError;

	bool savech = false, unget = false;
	int ch = 0;

	const int kTokenSize = 65;
	char buffer[kTokenSize];
	int index = 0;

	for (; state != ScannerStateDone;) {
		if (!reader_.GetNextChar(&ch)) {
			return false;
		}

		savech = true;
		unget = false;

		switch (state) {
		case ScannerStateStart:
			if (ch == ' ' || ch == '\t' || ch == '\n') {
				savech = false;
			}
			else if (isdigit(ch)) {
				state = ScannerStateNumber;
			}
			else if (isalpha(ch)) {
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

				case '(':
					tokenType = ScannerTokenLeftParenthesis;
					break;

				case ')':
					tokenType = ScannerTokenRightParenthesis;
					break;

				case ';':
					tokenType = ScannerTokenSemicolon;
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

		case ScannerStateComment:
			break;

		case ScannerStateID:
			if (!isdigit(ch) && !isalpha(ch)) {
				state = ScannerStateDone;
				tokenType = ScannerTokenID;
				unget = true;
			}
			break;

		case ScannerStateNumber:
			if (!isdigit(ch)) {
				state = ScannerStateDone;
				tokenType = ScannerTokenNumber;
				unget = true;
			}
			break;
		}

		if (saveCh) {
			if (index >= kTokenSize - 1) {
				printf("token length exceed");
				return false;
			}

			buffer[index++] = ch;
		}

		token->tokenType = tokenType;
		if (tokenType == ScannerTokenID) {
			token->token = AddConstant(buffer);
		}
		else if (tokenType == ScannerTokenNumber) {
			token = AddSymbol(buffer);
		}
	}
}
