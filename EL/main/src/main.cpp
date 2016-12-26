#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

#include "scanner.h"

int main() {
	Debug::EnableMemoryLeakCheck();

	TextScanner scanner;
	scanner.SetText("<< < ? ==<<<");
	char token[256];
	for (ScannerTokenType type; (type = scanner.GetToken(token)) != ScannerTokenEndOfFile;) {
		printf("%s\n", token);
	}

	Timer::Start();
	LanguageParameter lp;
	lp.productions = "main//config//lr_grammar.txt";

	Language* lang = new Language(&lp);
	Debug::Log(Utility::Format("%.2f seconds used to create parser.", Timer::Stop()));

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	if (lang->Parse(&tree, "main/debug/test.el")) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}

 	delete lang;

	return 0;
}
