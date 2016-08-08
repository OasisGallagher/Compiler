#include "../scanner/scanner.h"

FileReader::FileReader(const char* path) {
	ifs_ = new std::ifstream();
	ifs_->open(path);

	buffer_ = new char[kBufferSize];
}

FileReader::~FileReader() {
	ifs_->close();
	delete ifs_;

	delete[] buffer_;
}

bool FileReader::GetNextChar(int* ch) {
	if (current_ == nullptr || *current_ == 0) {
		if (!ReadBuffer()) {
			return false;
		}

		bool newline = false;

		if (current_ != nullptr) {
			*ch = '\n';
			newline = true;
		}

		current_ = buffer_;
		++lineNumber;
		linePosition = 0;

		if (newline) {
			return true;
		}
	}

	*ch = *current_++;
	++linePosition;
	return true;
}

bool FileReader::SkipLine() {
	current_ = buffer_;
	++lineNumber;
	linePosition = 0;
	return ReadBuffer();
}

void FileReader::UngetNextChar() {
	if (linePosition != 0) {
		--linePosition;
	}
}

bool FileReader::ReadBuffer() {
	return !!ifs_->getline(buffer_, kBufferSize);
}

TokenValueType NumberLiteralContainer::Add(const char* constant) {
	Container::iterator ite = cont_.find(constant);
	NumberLiteral* ans = nullptr;

	if (ite == cont_.end()) { 
		ans = new NumberLiteral();
		ans->value = atoi(constant);
		cont_[constant] = ans;
	}
	else {
		ans = ite->second;
	}

	return ans;
}

TokenValueType StringLiteralContainer::Add(const char* constant) {
	Container::iterator ite = cont_.find(constant);
	StringLiteral* ans = nullptr;

	if (ite == cont_.end()) {
		ans = new StringLiteral();
		ans->value = atoi(constant);
		cont_[constant] = ans;
	}
	else {
		ans = ite->second;
	}

	return ans;
}

TokenValueType SymbolContainer::Add(const char* symbol) {
	Container::iterator ite = cont_.find(symbol);
	Symbol* ans = nullptr;

	if (ite == cont_.end()) {
		ans = new Symbol();
		ans->name = symbol;
		cont_[symbol] = ans;
	}
	else {
		ans = ite->second;
	}

	return ans;
}

Scanner::Scanner(const char* path) {
	reader_ = new FileReader(path);

	symbols_ = new SymbolContainer();
	numberLiterals_ = new NumberLiteralContainer();
	stringLiterals_ = new StringLiteralContainer();
}

Scanner::~Scanner() {
	delete reader_;

	delete symbols_;
	delete numberLiterals_;
	delete stringLiterals_;
}

bool Scanner::GetToken(ScannerToken* token) {
	ScannerStateType state = ScannerStateStart;
	ScannerTokenType tokenType = ScannerTokenError;

	bool savech = false, unget = false, nextline = false;
	int ch = 0;

	const int kTokenSize = 65;
	char buffer[kTokenSize];
	int index = 0;

	for (; state != ScannerStateDone;) {
		if (!reader_->GetNextChar(&ch)) {
			return false;
		}

		savech = true;
		unget = false;
		nextline = false;

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
			else if (ch == '"') {
				state = ScannerStateDoubleQuotes;
				savech = false;
			}
			else if (ch == '#') {
				nextline = true;
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

		case ScannerStateDoubleQuotes:
			if (ch == '"') {
				tokenType = ScannerTokenString;
				savech = false;
				state = ScannerStateDone;
			}
			break;

		case ScannerStateID:
			if (!isdigit(ch) && !isalpha(ch)) {
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
			}
			break;
		}

		if (unget) {
			reader_->UngetNextChar();
		}

		if (savech) {
			if (index >= kTokenSize - 1) {
				printf("token length exceed");
				return false;
			}

			buffer[index++] = ch;
		}

		if (nextline && !reader_->SkipLine()) {
			return false;
		}
	}

	buffer[index] = 0;

	token->tokenType = tokenType;
	token->token = nullptr;

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

ScannerTokenType Scanner::GetReserveTokenType(const char* name) {
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
