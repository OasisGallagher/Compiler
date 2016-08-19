#include "scanner.h"
#include "parser.h"
#include "syntax_tree.h"
#include "debug.h"
#include "utilities.h"

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
	"$statement : $if_stmt | other",
	"$if_stmt : if($exp) $statement $else_part",
	"$else_part : else $statement | epsilon",
	"$exp : 0 | 1",
};

const char* grammar8[] = {
	"$exp : $exp + number | number"
};

#define SetLanguage(_Ans, _Prod) if (true) { _Ans.productions = _Prod; _Ans.nproductions = sizeof(_Prod) / sizeof(_Prod[0]); } else (void)0

int main() {
	LanguageParameter lp;

	SetLanguage(lp, grammar);
	Language lang(&lp);

	Debug::Log(lang.ToString());

	SyntaxTree* tree = nullptr;
	lang.Parse(&tree, "test.el");

	if (tree != nullptr) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree ", 48));
		Debug::Log(tree->ToString());
	}
	
	return 0;
}
