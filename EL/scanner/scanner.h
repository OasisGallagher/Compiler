#include <fstream>
#include <map>

typedef void* TokenValueType;

// Scannerɨ�赽�ı��.
enum ScannerTokenType {
	// ����.
	ScannerTokenError,
	// �ļ�����.
	ScannerTokenEndOfFile,

	// ������if.
	ScannerTokenIf,
	// ������then.
	ScannerTokenThen,
	// ������else.
	ScannerTokenElse,
	// ������end.
	ScannerTokenEnd,

	// ID(this, ���ű�ָ��).
	ScannerTokenID,
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

	// �ַ���.
	ScannerTokenString,

	// ������.
	ScannerTokenLeftParenthesis,
	// ������.
	ScannerTokenRightParenthesis,
	// �ֺ�.
	ScannerTokenSemicolon,
};

struct NumberLiteral {
	int value;
};

struct Symbol {
	std::string name;
};

struct StringLiteral {
	std::string value;
};

class NumberLiteralContainer {
public:
	TokenValueType Add(const char* number);

private:
	typedef std::map<std::string, NumberLiteral*> Container;
	Container cont_;
};

class StringLiteralContainer {
public:
	TokenValueType Add(const char* number);

private:
	typedef std::map<std::string, StringLiteral*> Container;
	Container cont_;
};

class SymbolContainer {
public:
	TokenValueType Add(const char* symbol);

private:
	typedef std::map<std::string, Symbol*> Container;
	Container cont_;
};

class FileReader {
public:
	FileReader(const char* path);
	~FileReader();

public:
	bool GetNextChar(int* ch);
	void UngetNextChar();
	bool SkipLine();

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

struct ScannerToken {
	ScannerTokenType tokenType;
	TokenValueType token;
};

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

class Scanner {
public:
	Scanner(const char* path);
	~Scanner();

	bool GetToken(ScannerToken* token);

private:
	ScannerStateType CheckCharInStartState(int ch, ScannerToken* token);
	ScannerTokenType GetReserveTokenType(const char* name);
private:
	FileReader* reader_;
	SymbolContainer* symbols_;
	NumberLiteralContainer* numberLiterals_;
	StringLiteralContainer* stringLiterals_;
};
