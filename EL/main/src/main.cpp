#include "main.h"
#include "debug.h"
#include "utilities.h"
#include "language.h"
#include "syntax_tree.h"

class Point {
public:
	void X(float x) {
		x_ = x;
	}
	void Y(float y) {
		y_ = y;
	}

	float X() const {
		return x_;
	}
	float Y() const {
		return y_;
	}
private:
	float x_, y_;
};

int main() {
	Debug::EnableMemoryLeakCheck();

	LanguageParameter lp;
	lp.productions = "main/config/lr_grammar.txt";

	Language* lang = new Language(&lp);

	Debug::Log(lang->ToString());

	SyntaxTree tree;

	/*if (lang->Parse(&tree, "main/debug/test.el")) {
		Debug::Log("\n" + Utility::Heading(" SyntaxTree "));
		Debug::Log(tree.ToString());
	}*/

 	delete lang;

	return 0;
}
