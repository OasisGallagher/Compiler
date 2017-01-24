#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

static const char* output = "main/config/output.bin";
static const char* testFile = "main/debug/test.js";
static const char* productions = "main/config/lr_grammar.txt";

int main(int argc, char** argv) {
	Debug::EnableMemoryLeakCheck();

	Language* lang = new Language;
	lang->Setup(productions, output);

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, testFile)) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
