#include <conio.h>
#include "syntax_tree.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"

const char* grammar[] = {
	"$exp : $exp + $term | $exp - $term | $term",
	"$term : $term * $factor | $factor",
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
	"$if_stmt : if($exp) $statement | if($exp) $statement else $statement",
	"$exp : 0 | 1",
};

const char* grammar8[] = {
	"$exp : $exp + number | number"
};

const char* grammar9[] = {
	"$E : $E + $T | $T",
	"$T : $T * $F | $F",
	"$F : ($E) | number",
};

const char* grammar10[] = {
	"$E : $E + $T | $T",
	"$T : $T * $F | $F",
	"$F : $P / $F | $P",
	"$P : ($E) | i",
};

#define SetLanguage(_Ans, _Prod) if (true) { _Ans.productions = _Prod; _Ans.nproductions = sizeof(_Prod) / sizeof(_Prod[0]); } else (void)0

int main() {
	Debug::EnableMemoryLeakCheck();

	LanguageParameter lp;

	SetLanguage(lp, grammar7);
	Language* lang = new Language(&lp);

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, "test.el")) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
