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
	"xor",
	"string",
	"leftparenthesis",
	"rightparenthesis"
	"semicolon",
};

const char* grammar[] = {
	"_exp : _exp _addop _term",
	"_exp : _term",
	"_addop : + | -",
	"_term : _term _mulop _factor",
	"_term : _factor",
	"_mulop : *",
	"_factor : (_exp)",
	"_factor : _number",
	"_number : 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9"
};

const char* grammar2[] = {
	"$A : $alphabet | $digits | epsilon"
};

const char* grammar3[] = {
	"$exp : $exp + $term | $exp - $term | $term",
};

#define GRAMMAR(name) name, sizeof(name) / sizeof(name[0])

int main() {
	GrammarParser parser;
	parser.Parse(GRAMMAR(grammar3));

	return 0;
}
