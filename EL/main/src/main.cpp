#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

static const char* productions = "main/config/lr_grammar.txt";

int main() {
	Debug::EnableMemoryLeakCheck();

	Debug::StartSample("create parser");
	Language* lang = new Language;
	lang->Setup(productions);
	Debug::EndSample();

	//Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, "main/debug/test.el")) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
