#include <fstream>
#include <map>

typedef void* TokenValueType;

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

	// 字符串.
	ScannerTokenString,

	// 左括号.
	ScannerTokenLeftParenthesis,
	// 右括号.
	ScannerTokenRightParenthesis,
	// 分号.
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
