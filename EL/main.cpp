#include "scanner.h"
#include "parser.h"
#include "debug.h"

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
	"seperator",
	"string",
	"leftparenthesis",
	"rightparenthesis"
	"semicolon",
};

const char* grammar[] = {
	"$exp : $exp $addop $term | $term",
	"$addop : + | -",
	"$term : $term $mulop $factor | $factor",
	"$mulop : *",
	"$factor : ($exp) | number",
};

const char* grammar2[] = {
	"$A : $letter | $digit | epsilon"
};

const char* grammar3[] = {
	"$exp : $exp + $term | $exp - $term | $term",
};

const char* grammar4[] = {
	"$A : $B $C | $D $E | $B $F"
};

const char* grammar5[] = {
	"$A : a b d | a b c | a b | a | c",
};

const char* grammar6[] = {
	"$exp : $term + $exp | $term",
};

const char* grammar7[] = {
	"$statement : $if_stmt | identifier",
	"$if_stmt : if($exp) $statement $else_part",
	"$else_part : else $statement | epsilon",
	"$exp : 0 | 1",
};

#define SetLanguage(_Ans, _Prod) if (true) { _Ans.productions = _Prod; _Ans.nproductions = sizeof(_Prod) / sizeof(_Prod[0]); } else (void)0

#include <fstream>

int main() {
	LanguageParameter lp;

	SetLanguage(lp, grammar);
	Language lang(&lp);

	Debug::Log(lang.ToString());

	std::ofstream ofs("debug.txt");
	ofs << lang.ToString() << std::endl;

	SyntaxTree* tree = nullptr;
	lang.Parse(&tree, "test.el");
	
	return 0;
}
