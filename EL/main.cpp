#include <conio.h>
#include <crtdbg.h>
#include "syntax_tree.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"

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

const char* grammar9[] = {
	"$E : $E + $T | $T",
	"$T : $T * $F | $F",
	"$F : ($E) | number",
};

#define SetLanguage(_Ans, _Prod) if (true) { _Ans.productions = _Prod; _Ans.nproductions = sizeof(_Prod) / sizeof(_Prod[0]); } else (void)0

static void EnableMemoryLeakCheck() {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

int main() {
	EnableMemoryLeakCheck();
	LanguageParameter lp;

	SetLanguage(lp, grammar9);
	Language* lang = new Language(&lp);

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, "test.el")) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree ", 48));
		Debug::Log(tree.ToString());
	}

	delete lang;

	return 0;
}
