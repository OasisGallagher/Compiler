#include "debug.h"
#include "reader.h"
#include "parser.h"
#include "scanner.h"
#include "language.h"
#include "syntaxer.h"
#include "lr_parser.h"

static const char* output = "main/config/output.bin";

Language::Language() {
	env_ = new Environment;
	syntaxer_ = new Syntaxer;
}

Language::~Language() {
	delete env_;
	delete syntaxer_;
}

void Language::Setup(const char* productions) {
	SetupEnvironment(productions);

	LRParser parser;
	parser.Setup(*syntaxer_, env_);
	syntaxer_->Save(output);
}

bool Language::Parse(SyntaxTree* tree, const std::string& file) {
	FileScanner scanner(file.c_str());
	return syntaxer_->ParseSyntax(tree, &scanner);
}

bool Language::SetupEnvironment(const char* productions) {
	NativeSymbols::Copy(env_->terminalSymbols, env_->nonterminalSymbols);

	if (!ParseProductions(productions)) {
		return false;
	}

	Assert(!env_->grammars.empty(), "grammar container is empty");
	Assert(env_->grammars.front()->GetLhs() == NativeSymbols::program, "invalid grammar. missing \"Program\".");

	return true;
}

GrammarSymbol Language::CreateSymbol(const std::string& text) {
	GrammarSymbolContainer* target = nullptr;
	if (Utility::IsTerminal(text)) {
		target = &env_->terminalSymbols;
	}
	else {
		target = &env_->nonterminalSymbols;
	}

	GrammarSymbolContainer::iterator ite = target->find(text);
	GrammarSymbol ans = nullptr;

	if (ite == target->end()) {
		ans = SymbolFactory::Create(text);
		target->insert(std::make_pair(text, ans));
	}
	else {
		ans = ite->second;
	}

	return ans;
}

bool Language::ParseProduction(TextScanner* textScanner, SymbolVector& symbols) {
	char token[MAX_TOKEN_CHARACTERS];

	for (ScannerTokenType tokenType; (tokenType = textScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		symbols.push_back(CreateSymbol(token));
	}

	Assert(symbols.front() != NativeSymbols::epsilon || symbols.size() == 1, "invalid epsilon usage.");

	return true;
}

bool Language::ParseProductions(const char* productions) {
	TextScanner textScanner;
	GrammarReader reader(productions);
	const GrammarTextContainer& cont = reader.GetGrammars();
	for (GrammarTextContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		const GrammarText& g = *ite;

		Grammar* grammar = new Grammar(CreateSymbol(g.lhs));
		env_->grammars.push_back(grammar);

		SymbolVector symbols;

		for (GrammarText::ProductionTextContainer::const_iterator ite2 = g.productions.begin(); ite2 != g.productions.end(); ++ite2) {
			const ProductionText& pr = *ite2;
			textScanner.SetText(pr.first.c_str());

			if (!ParseProduction(&textScanner, symbols)) {
				return false;
			}

			grammar->AddCondinate(pr.second, symbols);
			symbols.clear();
		}
	}

	return true;
}

std::string Language::ToString() const {
	return syntaxer_->ToString();
}
