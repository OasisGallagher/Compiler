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
	"$factor : ($exp) | $digit",
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
	"$statement : $if_stmt | other",
	"$if_stmt : if($exp) $statement $else_part",
	"$else_part : else $statment | epsilon",
	"$exp : 0 | 1",
};

#define FORMAT_GRAMMARS(name) name, sizeof(name) / sizeof(name[0])

int main() {
	Language lang;
	lang.SetGrammars(FORMAT_GRAMMARS(grammar));
	Debug::Log(lang.ToString());
	return 0;
}
