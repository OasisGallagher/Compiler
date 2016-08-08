#include "scanner/scanner.h"
const char* tokenName[] = {
	"err",
	"eof",
	"if",
	"then",
	"else",
	"end",
	"id",
	"number",
	"assign",
	"equal",
	"less",
	"lessequal",
	"greater",
	"greaterequal",
	"plus",
	"minus",
	"multiply",
	"divide",
	"string",
	"leftparenthesis",
	"rightparenthesis"
	"semicolon",
};

int main() {
	Scanner* scanner = new Scanner("test.el");
	ScannerToken token;

	for (; scanner->GetToken(&token);) {
		printf("type = %s, value = %p\n", tokenName[token.tokenType], token.token);
	}

	delete scanner;
	return 0;
}
