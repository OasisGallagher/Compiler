#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

static const char* demo = "main/debug/demo.js";
static const char* compiler = "main/config/compiler";
static const char* productions = "main/config/lr_grammar.txt";

int main(int argc, char** argv) {
	Debug::EnableMemoryLeakCheck();

	Language* lang = new Language;
	lang->Setup(productions, compiler);

	//Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, demo)) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
