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
	"$statement : if($exp) $statement | if($exp) $statement else $statement | other",
	"$exp : number",
};

/*
	"$statement : $if_stmt | other",
	"$if_stmt : if($exp) $statement $else_part",
	"$else_part : else $statement | epsilon",
	"$exp : 0 | 1",
*/

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

const char* grammar11[] = {	// LL(1) Grammar.
	"$program : $stmt_seq",
	"$stmt_seq : $stmt_seq ; $stmt | $stmt | $stmt_seq newline $stmt",
	"$stmt : $if_stmt | $repeat_stmt | $assign_stmt | $read_stmt | $write_stmt | epsilon",
	"$if_stmt : if $exp then $stmt_seq end | if $exp then $stmt_seq else $stmt_seq end",
	"$repeat_stmt : repeat $stmt_seq until $exp",
	"$assign_stmt : identifier = $exp",
	"$read_stmt : read identifier",
	"$write_stmt : write identifier",
	"$exp : $simple_exp < $simple_exp | $simple_exp <= $simple_exp | $simple_exp == $simple_exp | $simple_exp >= $simple_exp | $simple_exp > $simple_exp | $simple_exp",
	"$simple_exp : $simple_exp + $term | $simple_exp - $term | $term",
	"$term : $term * $factor | $term / $factor | $factor",
	"$factor : ( $exp ) | number | identifier",
};

const char* grammar12[] = {	// Operator Precedence Grammar.
	"$program"
		": $stmt_seq",
	"$stmt_seq"
		": $stmt_seq ;"
		"| newline $stmt_seq"
		"| $stmt_seq newline"
		"| $stmt_seq ; $stmt"
		"| $stmt_seq newline $stmt"
		"| $stmt",
	"$function_def"
		": function identifier ( $argument_list ) end"
		"| function identifier ( $argument_list ) $stmt_seq end",
	"$argument_list"
		": identifier"
		"| $argument_list , identifier",
	"$stmt"
		": $if_stmt"
		"| $repeat_stmt"
		"| $assign_stmt"
		"| $read_stmt"
		"| $write_stmt"
		"| $empty_stmt",
	"$empty_stmt"
		": ;",
	"$if_stmt"
		": if $exp then end"
		"| if $exp then $stmt_seq end"
		"| if $exp then else end"
		"| if $exp then else $stmt_seq end"
		"| if $exp then $stmt_seq else end"
		"| if $exp then $stmt_seq else $stmt_seq end",
	"$repeat_stmt"
		": repeat until $exp"
		"| repeat $stmt_seq until $exp",
	"$assign_stmt"
		": identifier = $exp",
	"$read_stmt"
		": read identifier",
	"$write_stmt"
		": write identifier",
	"$exp" 
		": $simple_exp < $simple_exp"
		"| $simple_exp <= $simple_exp"
		"| $simple_exp == $simple_exp"
		"| $simple_exp >= $simple_exp"
		"| $simple_exp > $simple_exp"
		"| $simple_exp",
	"$simple_exp"
		": $simple_exp + $term"
		"| $simple_exp - $term"
		"| $term",
	"$term"
		": $term * $factor"
		"| $term / $factor"
		"| $factor",
	"$factor"
		": ( $exp )"
		"| number"
		"| identifier",
};

#define SetLanguage(_Ans, _Prod) if (true) { _Ans.productions = _Prod; _Ans.nproductions = sizeof(_Prod) / sizeof(_Prod[0]); } else (void)0

int main() {
	Debug::EnableMemoryLeakCheck();

	LanguageParameter lp;

	SetLanguage(lp, grammar12);
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
