#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

static const char* output = "main/config/output.bin";
static const char* productions = "main/config/lr_grammar.txt";

int main(int argc, char** argv) {
	Debug::EnableMemoryLeakCheck();

	Language* lang = new Language;
	lang->Setup(productions, output);

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	const char* filePath = "main/debug/test.el";
	if (argc > 1) {
		filePath = argv[1];
	}

	if (lang->Parse(&tree, filePath)) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
