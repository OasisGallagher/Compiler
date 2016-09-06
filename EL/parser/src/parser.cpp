#include <sstream>

#include "parser.h"
#include "scanner.h"

Parser::Parser() {
	InitializeTerminalSymbolContainer();
}

Parser::~Parser() {
	DestroyGammars();
}

void Parser::InitializeTerminalSymbolContainer() {
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::zero.ToString(), GrammarSymbol::zero));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::number.ToString(), GrammarSymbol::number));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::string.ToString(), GrammarSymbol::string));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::epsilon.ToString(), GrammarSymbol::epsilon));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::identifier.ToString(), GrammarSymbol::identifier));
}

GrammarSymbol Parser::CreateSymbol(const std::string& text) {
	GrammarSymbolContainer* target = nullptr;
	if (Utility::IsTerminal(text)) {
		target = &terminalSymbols_;
	}
	else {
		target = &nonterminalSymbols_;
	}

	GrammarSymbolContainer::iterator ite = target->find(text);
	GrammarSymbol ans;

	if (ite == target->end()) {
		ans = SymbolFactory::Create(text);
		target->insert(std::make_pair(text, ans));
	}
	else {
		ans = ite->second;
	}

	return ans;
}

bool Parser::SetGrammars(const char** productions, int count) {
	Clear();

	TextScanner textScanner;
	for (int i = 0; i < count; ++i) {
		textScanner.SetText(productions[i]);
		if (!ParseProductions(&textScanner)) {
			return false;
		}
	}

	return ParseGrammars();
}

Grammar* Parser::FindGrammar(const GrammarSymbol& lhs) {
	Grammar* g = nullptr;
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		if ((*ite)->GetLhs() == lhs) {
			g = *ite;
			break;
		}
	}

	return g;
}

GrammarSymbol Parser::FindSymbol(const ScannerToken& token) {
	GrammarSymbol answer = GrammarSymbol::null;
	if (token.tokenType == ScannerTokenEndOfFile) {
		answer = GrammarSymbol::zero;
	}
	else if (token.tokenType == ScannerTokenNumber) {
		answer = GrammarSymbol::number;
	}
	else if (token.tokenType == ScannerTokenString) {
		answer = GrammarSymbol::string;
	}
	else {
		if (Utility::IsTerminal(token.text)) {
			GrammarSymbolContainer::const_iterator pos = terminalSymbols_.find(token.text);
			if (pos != terminalSymbols_.end()) {
				answer = pos->second;
			}
			else {
				answer = GrammarSymbol::identifier;
			}
		}
		else {
			GrammarSymbolContainer::const_iterator ite = nonterminalSymbols_.find(token.text);
			if (ite != nonterminalSymbols_.end()) {
				answer = ite->second;
			}
		}
	}

	return answer;
}

bool Parser::MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src) {
	bool modified = false;
	for (GrammarSymbolSet::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		if (*ite != GrammarSymbol::epsilon) {
			modified = dest.insert(*ite).second || modified;
		}
	}

	return modified;
}

bool Parser::ParseProductions(TextScanner* textScanner) {
	char token[MAX_TOKEN_CHARACTERS];

	ScannerTokenType tokenType = textScanner->GetToken(token);
	Assert(tokenType != ScannerTokenEndOfFile, "invalid production. missing left hand side.");

	Grammar* grammar = new Grammar(CreateSymbol(token));

	Condinate cond;

	textScanner->GetToken(token);
	Assert(strcmp(token, ":") == 0, "invalid production. missing \":\".");

	for (; (tokenType = textScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		if (tokenType == ScannerTokenSign && strcmp(token, "|") == 0) {
			Assert(!cond.empty(), "empty production");
			grammar->AddCondinate(cond);
			cond.clear();
			continue;
		}

		cond.push_back(CreateSymbol(token));
	}

	grammar->AddCondinate(cond);

	grammars_.push_back(grammar);

	return true;
}

void Parser::Clear() {
	DestroyGammars();

	terminalSymbols_.clear();
	InitializeTerminalSymbolContainer();

	nonterminalSymbols_.clear();
}

void Parser::DestroyGammars() {
	for (GrammarContainer::iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		delete *ite;
	}

	grammars_.clear();
}

std::string Parser::ToString() const {
	std::ostringstream oss;
	oss << Utility::Heading(" Grammars ") << "\n";
	
	const char* newline = "";
	for (GrammarContainer::const_iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		oss << newline;
		oss << (*ite)->ToString();
		newline = "\n";
	}

	oss << "\n\n";

	oss << Utility::Heading(" TerminalSymbols ") << "\n";
	oss << terminalSymbols_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" NonterminalSymbols ") << "\n";
	oss << nonterminalSymbols_.ToString();

	return oss.str();
}
