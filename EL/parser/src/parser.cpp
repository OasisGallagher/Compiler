#include <sstream>

#include "table.h"
#include "reader.h"
#include "parser.h"
#include "action.h"
#include "scanner.h"

Parser::Parser() {
	symTable_ = new SymTable();
	literalTable_ = new LiteralTable();
	constantTable_ = new ConstantTable();

	actionParser_ = new ActionParser();

	InitializeTerminalSymbolContainer();
}

Parser::~Parser() {
	DestroyGammars();

	delete symTable_;
	delete literalTable_;
	delete constantTable_;

	delete actionParser_;
}

void Parser::InitializeTerminalSymbolContainer() {
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::zero.ToString(), GrammarSymbol::zero));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::number.ToString(), GrammarSymbol::number));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::string.ToString(), GrammarSymbol::string));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::epsilon.ToString(), GrammarSymbol::epsilon));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::newline.ToString(), GrammarSymbol::newline));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::positive.ToString(), GrammarSymbol::positive));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::negative.ToString(), GrammarSymbol::negative));
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

bool Parser::SetGrammars(const char* productions) {
	Clear();

	TextScanner textScanner;
	GrammarReader reader(productions);
	const GrammarDefContainer& cont = reader.GetGrammars();
	for (GrammarDefContainer::const_iterator ite = cont.begin();
		ite != cont.end(); ++ite) {
		const GrammarDef& g = *ite;
		
		Grammar* grammar = new Grammar(CreateSymbol(g.lhs));
		grammars_.push_back(grammar);

		Condinate cond;

		for (GrammarDef::ProductionDefContainer::const_iterator ite2 = g.productions.begin(); ite2 != g.productions.end(); ++ite2) {
			const ProductionDef& pr = *ite2;
			
			textScanner.SetText(pr.first.c_str());
			
			if (!ParseProductions(&textScanner, cond.symbols)) {
				return false;
			}

			cond.action = actionParser_->Parse(pr.second);
			grammar->AddCondinate(cond);

			cond.action = nullptr;
			cond.symbols.clear();
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

GrammarSymbol Parser::FindSymbol(const ScannerToken& token, void*& addr) {
	addr = nullptr;

	GrammarSymbol answer = GrammarSymbol::null;
	if (token.tokenType == ScannerTokenEndOfFile) {
		answer = GrammarSymbol::zero;
	}
	else if (token.tokenType == ScannerTokenNumber) {
		answer = GrammarSymbol::number;
		addr = constantTable_->Add(token.text);
	}
	else if (token.tokenType == ScannerTokenString) {
		answer = GrammarSymbol::string;
		addr = literalTable_->Add(token.text);
	}
	else if (token.tokenType == ScannerTokenNewline) {
		answer = GrammarSymbol::newline;
	}
	else if (token.tokenType == ScannerTokenPositive) {
		answer = GrammarSymbol::positive;
	}
	else if (token.tokenType == ScannerTokenNegative) {
		answer = GrammarSymbol::negative;
	}
	else if (Utility::IsTerminal(token.text)) {
		GrammarSymbolContainer::const_iterator pos = terminalSymbols_.find(token.text);
		if (pos != terminalSymbols_.end()) {
			answer = pos->second;
		}
		else {
			answer = GrammarSymbol::identifier;
			addr = symTable_->Add(token.text);
		}
	}
	else {
		GrammarSymbolContainer::const_iterator ite = nonterminalSymbols_.find(token.text);
		if (ite != nonterminalSymbols_.end()) {
			answer = ite->second;
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

bool Parser::ParseProductions(TextScanner* textScanner, SymbolVector& symbols) {
	char token[MAX_TOKEN_CHARACTERS];

	for (ScannerTokenType tokenType; (tokenType = textScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		symbols.push_back(CreateSymbol(token));
	}

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
