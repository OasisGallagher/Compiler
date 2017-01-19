#include <sstream>

#include "debug.h"
#include "table.h"
#include "reader.h"
#include "parser.h"
#include "action.h"
#include "scanner.h"

Parser::Parser() {
}

Parser::~Parser() {
}

bool Parser::Setup(Syntaxer& syntaxer, Environment* env) {
	env_ = env;
	CreateFirstSets();
	CreateFollowSets();

	return ParseGrammars(syntaxer, env);
}

bool Parser::MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src) {
	bool modified = false;
	for (GrammarSymbolSet::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		if (*ite != NativeSymbols::epsilon) {
			modified = dest.insert(*ite).second || modified;
		}
	}

	return modified;
}


void Parser::Clear() {
}

void Parser::CreateFirstSets() {
	for (GrammarSymbolContainer::const_iterator ite = env_->terminalSymbols.begin(); ite != env_->terminalSymbols.end(); ++ite) {
		firstSetContainer_[ite->second].insert(ite->second);
	}

	for (; CreateFirstSetsOnePass();) {
	}
}

bool Parser::CreateFirstSetsOnePass() {
	bool anySetModified = false;

	for (GrammarContainer::const_iterator ite = env_->grammars.begin(); ite != env_->grammars.end(); ++ite) {
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
				if (currentFirstSet.find(NativeSymbols::epsilon) == currentFirstSet.end()) {
					break;
				}
			}

			if (ite3 == c->symbols.end()) {
				anySetModified = firstSet.insert(NativeSymbols::epsilon).second || anySetModified;
			}
		}
	}

	return anySetModified;
}

void Parser::CreateFollowSets() {
	followSetContainer_[env_->grammars.front()->GetLhs()].insert(NativeSymbols::zero);

	for (; CreateFollowSetsOnePass();) {
	}
}

bool Parser::CreateFollowSetsOnePass() {
	bool anySetModified = false;

	GrammarSymbolSet gss;
	for (GrammarContainer::const_iterator ite = env_->grammars.begin(); ite != env_->grammars.end(); ++ite) {
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

				if (gss.find(NativeSymbols::epsilon) != gss.end()) {
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
	for (GrammarContainer::const_iterator ite = env_->grammars.begin(); ite != env_->grammars.end(); ++ite) {
		oss << newline;
		oss << (*ite)->ToString();
		newline = "\n";
	}

	oss << "\n\n";

	oss << Utility::Heading(" TerminalSymbols ") << "\n";
	oss << env_->terminalSymbols.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" NonterminalSymbols ") << "\n";
	oss << env_->nonterminalSymbols.ToString();

	return oss.str();
}
