#include "scanner.h"
#include "language.h"
#include "ll_parser.h"

Language::Language(LanguageParameter* parameter) {
	parser_ = new LLParser(parameter->productions, parameter->nproductions);
}

Language::~Language() {
	delete parser_;
}

bool Language::Parse(SyntaxTree* tree, const std::string& file) {
	FileScanner scanner(file.c_str());
	return parser_->ParseFile(tree, &scanner);
}

std::string Language::ToString() const {
	return parser_->ToString();
}
