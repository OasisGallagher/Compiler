#include <sstream>

#include "debug.h"
#include "table.h"
#include "reader.h"
#include "parser.h"
#include "action.h"
#include "scanner.h"

Parser::Parser() {
	symTable_ = new SymTable();
	literalTable_ = new LiteralTable();
	constantTable_ = new ConstantTable();

	InitializeTerminalSymbolContainer();
}

Parser::~Parser() {
	DestroyGammars();

	delete symTable_;
	delete literalTable_;
	delete constantTable_;
}

void Parser::InitializeTerminalSymbolContainer() {
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::zero.ToString(), GrammarSymbol::zero));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::number.ToString(), GrammarSymbol::number));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::string.ToString(), GrammarSymbol::string));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::epsilon.ToString(), GrammarSymbol::epsilon));
	terminalSymbols_.insert(std::make_pair(GrammarSymbol::identifier.ToString(), GrammarSymbol::identifier));

	nonterminalSymbols_.insert(std::make_pair(GrammarSymbol::program.ToString(), GrammarSymbol::program));
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

bool Parser::SetGrammars(const char* productions) {
	TextScanner textScanner;
	GrammarReader reader(productions);
	const GrammarTextContainer& cont = reader.GetGrammars();
	int test = 0;
	for (GrammarTextContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		const GrammarText& g = *ite;
		
		Grammar* grammar = new Grammar(CreateSymbol(g.lhs));
		grammars_.push_back(grammar);

		SymbolVector symbols;

		for (GrammarText::ProductionTextContainer::const_iterator ite2 = g.productions.begin(); ite2 != g.productions.end(); ++ite2) {
			const ProductionText& pr = *ite2;
			++test;
			textScanner.SetText(pr.first.c_str());
			
			if (!ParseProductions(&textScanner, symbols)) {
				return false;
			}

			grammar->AddCondinate(pr.second, symbols);
			symbols.clear();
		}
	}

	Assert(!grammars_.empty(), "grammar container is empty");
	Assert(grammars_.front()->GetLhs() == GrammarSymbol::program, "invalid grammar. missing \"Program\".");

	CreateFirstSets();
	CreateFollowSets();

	return ParseGrammars();
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

	if (!answer) {
		__asm int 3
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

	Assert(symbols.front() != GrammarSymbol::epsilon || symbols.size() == 1, "invalid epsilon usage.");

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

void Parser::CreateFirstSets() {
	for (GrammarSymbolContainer::const_iterator ite = terminalSymbols_.begin(); ite != terminalSymbols_.end(); ++ite) {
		firstSetContainer_[ite->second].insert(ite->second);
	}

	for (; CreateFirstSetsOnePass();) {
	}
}

bool Parser::CreateFirstSetsOnePass() {
	bool anySetModified = false;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		GrammarSymbolSet& firstSet = firstSetContainer_[g->GetLhs()];

		for (CondinateContainer::const_iterator ite2 = conds.begin(); ite2 != conds.end(); ++ite2) {
			Condinate* c = *ite2;
			SymbolVector::iterator ite3 = c->symbols.begin();

			for (; ite3 != c->symbols.end(); ++ite3) {
				GrammarSymbol& current = *ite3;

				anySetModified = MergeNonEpsilonElements(firstSet, firstSetContainer_[current]) || anySetModified;

				GrammarSymbolSet& currentFirstSet = firstSetContainer_[current];
				if (currentFirstSet.find(GrammarSymbol::epsilon) == currentFirstSet.end()) {
					break;
				}
			}

			if (ite3 == c->symbols.end()) {
				anySetModified = firstSet.insert(GrammarSymbol::epsilon).second || anySetModified;
			}
		}
	}

	return anySetModified;
}

void Parser::CreateFollowSets() {
	followSetContainer_[grammars_.front()->GetLhs()].insert(GrammarSymbol::zero);

	for (; CreateFollowSetsOnePass();) {
	}
}

bool Parser::CreateFollowSetsOnePass() {
	bool anySetModified = false;

	GrammarSymbolSet gss;
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();

		for (CondinateContainer::const_iterator ite2 = conds.begin(); ite2 != conds.end(); ++ite2) {
			Condinate* current = *ite2;
			for (SymbolVector::iterator ite3 = current->symbols.begin(); ite3 != current->symbols.end(); ++ite3) {
				GrammarSymbol& symbol = *ite3;
				if (symbol.SymbolType() == GrammarSymbolTerminal) {
					continue;
				}

				SymbolVector::iterator ite4 = ite3;
				firstSetContainer_.GetFirstSet(gss, ++ite4, current->symbols.end());
				anySetModified = MergeNonEpsilonElements(followSetContainer_[symbol], gss) || anySetModified;

				if (gss.find(GrammarSymbol::epsilon) != gss.end()) {
					anySetModified = MergeNonEpsilonElements(followSetContainer_[symbol], followSetContainer_[g->GetLhs()]) || anySetModified;
				}

				gss.clear();
			}
		}
	}

	return anySetModified;
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
