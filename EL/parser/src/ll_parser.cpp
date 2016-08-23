#include <stack>
#include <sstream>
#include <algorithm>
#include <functional>

#include "token.h"
#include "scanner.h"
#include "ll_parser.h"
#include "syntax_tree.h"
#include "parsing_table.h"

LLParser::LLParser(const char** productions, int nproductions) {
	parsingTable_ = new ParsingTable();
	SetGrammars(productions, nproductions);
}

LLParser::~LLParser() {
	delete parsingTable_;
	for (GrammarContainer::iterator ite = grammars_.begin();
		ite != grammars_.end(); ++ite) {
		delete *ite;
	}
}

bool LLParser::SetGrammars(const char** productions, int count) {
	LineScanner lineScanner;
	for (int i = 0; i < count; ++i) {
		lineScanner.SetText(productions[i]);
		if (!ParseProductions(&lineScanner)) {
			return false;
		}
	}

	return ParseGrammars();
}

bool LLParser::ParseGrammars() {
	RemoveLeftRecursion();
	LeftFactoring();

	CreateFirstSets();
	CreateFollowSets();

	return CreateParsingTable();
}

std::string LLParser::ToString() const {
	const int headingLength = 48;
	const char* newline = "";
	std::ostringstream oss;
	oss << Utility::Heading(" LL(1)Grammars ", headingLength) << "\n";

	for (GrammarContainer::const_iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		oss << newline;
		oss << (*ite)->ToString();
		newline = "\n";
	}

	oss << "\n\n";

	oss << Utility::Heading(" BuildinSymbols ", headingLength) << "\n";
	oss << buildinSymbols_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" First ", headingLength) << "\n";
	oss << firstSetContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" Follow ", headingLength) << "\n";
	oss << followSetContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" ParsingTable ", headingLength) << "\n";
	oss << parsingTable_->ToString();

	return oss.str();
}

void LLParser::RemoveLeftRecursion() {
	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end();) {
		if (RemoveImmidiateLeftRecursion(*ite, &newGrammars)) {
			ite = grammars_.erase(ite);
			grammars_.insert(ite, newGrammars.begin(), newGrammars.end());
			newGrammars.clear();
		}
		else {
			++ite;
		}
	}
}

bool LLParser::RemoveImmidiateLeftRecursion(Grammar* g, GrammarContainer* newGrammars) {
	const GrammarSymbol& left = g->GetLeft();
	const CondinateContainer& condinates = g->GetCondinates();

	CondinateContainer::const_iterator pos = condinates.begin();
	for (; pos != condinates.end(); ++pos) {
		if ((*pos)->front() != left) {
			break;
		}
	}

	if (pos == condinates.begin()) {
		return false;
	}

	Assert(pos != condinates.end(), "invalid production");
	Grammar* grammar = new Grammar(left);

	GrammarSymbol left2 = new NonterminalSymbol((left.ToString() + "_2"));
	Grammar* grammar2 = new Grammar(left2);
	Condinate cond;

	for (CondinateContainer::const_iterator ite = pos; ite != condinates.end(); ++ite) {
		cond.insert(cond.end(), (*ite)->begin(), (*ite)->end());
		cond.push_back(left2);

		grammar->AddCondinate(cond);
		cond.clear();
	}

	for (CondinateContainer::const_iterator ite = condinates.begin(); ite != pos; ++ite) {
		cond.insert(cond.end(), (*ite)->begin() + 1, (*ite)->end());
		cond.push_back(left2);

		grammar2->AddCondinate(cond);
		cond.clear();
	}

	cond.push_back(GrammarSymbol::epsilon);
	grammar2->AddCondinate(cond);

	newGrammars->push_back(grammar);
	newGrammars->push_back(grammar2);

	return true;
}

int LLParser::LongestCommonPrefix(const Condinate* first, const Condinate* second) {
	int size = std::min(first->size(), second->size());
	int ans = 0;
	for (int i = 0; i < size; ++i) {
		if (first->at(i) != second->at(i)) {
			break;
		}

		++ans;
	}

	return ans;
}

void LLParser::LeftFactoring() {
	std::for_each(grammars_.begin(), grammars_.end(), std::mem_fun(&Grammar::SortCondinates));

	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end();) {
		if (LeftFactoringOnGrammar(*ite, &newGrammars)) {
			delete *ite;
			ite = grammars_.erase(ite);
			grammars_.insert(ite, newGrammars.begin(), newGrammars.end());
			newGrammars.clear();
		}
		else {
			++ite;
		}
	}
}

bool LLParser::CalculateLongestFactor(Grammar* g, int* range, int* length) {
	const CondinateContainer& cond = g->GetCondinates();
	int maxlength = 0;
	int from = 0, index = 1;
	for (; index < (int)cond.size(); ++index) {
		int length = LongestCommonPrefix(cond[index - 1], cond[index]);

		if (length < maxlength) {
			break;
		}

		if (length > maxlength) {
			from = index - 1;
			maxlength = length;
		}
	}

	if (maxlength == 0) {
		return false;
	}

	*length = maxlength;
	*range = Utility::MakeDword(from, index);
	return true;
}

bool LLParser::LeftFactoringOnGrammar(Grammar* g, GrammarContainer* newGrammars) {
	int range, length, nsindex = 1;
	Grammar* seed = g;
	for (; CalculateLongestFactor(g, &range, &length);) {
		int from = Utility::Loword(range), to = Utility::Highword(range);
		Grammar* grammar = new Grammar(g->GetLeft());

		GrammarSymbol left2 = new NonterminalSymbol((g->GetLeft().ToString() + "_" + std::to_string(++nsindex)));

		Condinate cond;
		const CondinateContainer& oldCondinates = g->GetCondinates();
		Condinate::iterator first = oldCondinates[from]->begin();
		Condinate::iterator last = first;
		std::advance(last, length);
		cond.insert(cond.end(), first, last);

		cond.push_back(left2);
		grammar->AddCondinate(cond);

		for (int i = 0; i < from; ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		for (int i = to; i < (int)oldCondinates.size(); ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		Grammar* grammar2 = new Grammar(left2);
		for (int i = from; i < to; ++i) {
			Assert(length <= (int)oldCondinates[i]->size(), "logic error");
			first = oldCondinates[i]->begin();
			std::advance(first, length);
			cond.clear();
			if (first != oldCondinates[i]->end()) {
				cond.insert(cond.end(), first, oldCondinates[i]->end());
				grammar2->AddCondinate(cond);
			}
			else {
				cond.push_back(GrammarSymbol::epsilon);
				grammar2->AddCondinate(cond);
			}
		}

		if (g != seed) {
			delete g;
		}

		newGrammars->push_front(grammar2);

		g = grammar;
		g->SortCondinates();
	}

	if (g != seed) {
		newGrammars->push_front(g);
		return true;
	}

	return false;
}

bool LLParser::ParseProductions(LineScanner* lineScanner) {
	char token[Constants::kMaxTokenCharacters];

	ScannerTokenType tokenType = lineScanner->GetToken(token);
	Assert(tokenType != ScannerTokenEndOfFile, "invalid production. missing left part.");

	Grammar* grammar = new Grammar(symbolContainer_.AddSymbol(token, false));

	Condinate cond;

	lineScanner->GetToken(token);
	Assert(strcmp(token, ":") == 0, "invalid production. missing \":\".");

	for (; (tokenType = lineScanner->GetToken(token)) != ScannerTokenEndOfFile;) {
		if (tokenType == ScannerTokenSign && strcmp(token, "|") == 0) {
			Assert(!cond.empty(), "empty production");
			grammar->AddCondinate(cond);
			cond.clear();
			continue;
		}

		GrammarSymbol symbol;
		if (tokenType != ScannerTokenSign && IsTerminal(token)) {
			BuildinSymbolContainer::iterator pos = buildinSymbols_.find(token);
			if (pos == buildinSymbols_.end()) {
				symbol = new TerminalSymbol(token);
				buildinSymbols_[token] = symbol;
			}
			else {
				symbol = pos->second;
			}
		}
		else {
			symbol = symbolContainer_.AddSymbol(token, IsTerminal(token));
		}

		cond.push_back(symbol);
	}

	grammar->AddCondinate(cond);

	grammars_.push_back(grammar);

	return true;
}

bool LLParser::MergeNonEpsilonElements(GrammarSymbolSet& dest, const GrammarSymbolSet& src) {
	bool modified = false;
	for (GrammarSymbolSet::const_iterator ite = src.begin(); ite != src.end(); ++ite) {
		if (*ite != GrammarSymbol::epsilon) {
			modified = dest.insert(*ite).second || modified;
		}
	}

	return modified;
}

void LLParser::CreateFirstSets() {
	for (; CreateFirstSetsOnePass();) {
	}
}

bool LLParser::CreateFirstSetsOnePass() {
	bool anySetModified = false;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		GrammarSymbolSet& firstSet = firstSetContainer_[g->GetLeft()];

		for (CondinateContainer::const_iterator ite2 = conds.begin(); ite2 != conds.end(); ++ite2) {
			Condinate* c = *ite2;
			GrammarSymbol& front = c->front();

			if (front.SymbolType() == GrammarSymbolTerminal) {
				anySetModified = firstSet.insert(c->front()).second || anySetModified;
				continue;
			}

			Condinate::iterator ite3 = c->begin();
			for (; ite3 != c->end(); ++ite3) {
				GrammarSymbol& current = *ite3;
				if (current.SymbolType() != GrammarSymbolNonterminal) {
					break;
				}

				anySetModified = MergeNonEpsilonElements(firstSet, firstSetContainer_[front]) || anySetModified;

				GrammarSymbolSet& currentFirstSet = firstSetContainer_[current];
				if (currentFirstSet.find(GrammarSymbol::epsilon) == currentFirstSet.end()) {
					break;
				}
			}

			if (ite3 == c->end()) {
				anySetModified = firstSet.insert(GrammarSymbol::epsilon).second || anySetModified;
			}
		}
	}

	return anySetModified;
}

void LLParser::CreateFollowSets() {
	followSetContainer_[grammars_.front()->GetLeft()].insert(GrammarSymbol::zero);

	for (; CreateFollowSetsOnePass();) {
	}
}

bool LLParser::CreateFollowSetsOnePass() {
	bool anySetModified = false;

	GrammarSymbolSet gss;
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();

		for (CondinateContainer::const_iterator ite2 = conds.begin(); ite2 != conds.end(); ++ite2) {
			Condinate* current = *ite2;
			for (Condinate::iterator ite3 = current->begin(); ite3 != current->end(); ++ite3) {
				GrammarSymbol& symbol = *ite3;
				if (symbol.SymbolType() == GrammarSymbolTerminal) {
					continue;
				}

				Condinate::iterator ite4 = ite3;
				GetFirstSet(&gss, ++ite4, current->end());
				anySetModified = MergeNonEpsilonElements(followSetContainer_[symbol], gss) || anySetModified;

				if (gss.find(GrammarSymbol::epsilon) != gss.end()) {
					anySetModified = MergeNonEpsilonElements(followSetContainer_[symbol], followSetContainer_[g->GetLeft()]) || anySetModified;
				}

				gss.clear();
			}
		}
	}

	return anySetModified;
}

bool LLParser::CreateParsingTable() {
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		if (!BuildParingTable(*ite)) {
		}
	}

	//AddAsyncSymbol();
	return true;
}

bool LLParser::BuildParingTable(Grammar* g) {
	const GrammarSymbol& left = g->GetLeft();
	const CondinateContainer& conds = g->GetCondinates();
	GrammarSymbolSet firstSet;

	for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
		Condinate* c = *ite;
		GetFirstSet(&firstSet, c->begin(), c->end());
		for (GrammarSymbolSet::iterator ite2 = firstSet.begin(); ite2 != firstSet.end(); ++ite2) {
			ParsingTable::iterator pos = parsingTable_->find(left, *ite2);
			if (pos != parsingTable_->end()) {
				std::string slot = "[" + pos->first.first.ToString() + ", " + pos->first.second.ToString() + "]";
				std::string p1 = pos->second.first.ToString() + " : " + pos->second.second->ToString();
				std::string p2 = left.ToString() + " : " + c->ToString();
				Debug::LogWarning("invalid LL(1) grammar at " + slot + "\n(1) " + p1 + "\n(2) " + p2);
			}
			else {
				parsingTable_->at(left, *ite2) = std::make_pair(left, c);
			}
		}

		if (firstSet.find(GrammarSymbol::epsilon) != firstSet.end()) {
			const GrammarSymbolSet& follow = followSetContainer_[left];
			for (GrammarSymbolSet::const_iterator ite3 = follow.begin(); ite3 != follow.end(); ++ite3) {
				ParsingTable::iterator pos = parsingTable_->find(left, *ite3);
				if (pos != parsingTable_->end()) {
					std::string slot = "[" + pos->first.first.ToString() + ", " + pos->first.second.ToString() + "]";
					std::string p1 = pos->second.first.ToString() + " : " + pos->second.second->ToString();
					std::string p2 = left.ToString() + " : " + c->ToString();
					Debug::LogWarning("invalid LL(1) grammar at " + slot + "\n(1) " + p1 + "\n(2) " + p2);
				}
				else {
					parsingTable_->at(left, *ite3) = std::make_pair(left, c);
				}
			}
		}

		firstSet.clear();
	}

	return true;
}

void LLParser::AddAsyncSymbol() {
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const GrammarSymbol& left = g->GetLeft();
		GrammarSymbolSetTable::const_iterator pos = followSetContainer_.find(left);
		if (pos == followSetContainer_.end()) {
			continue;
		}

		const GrammarSymbolSet& follow = pos->second;
		for (GrammarSymbolSet::const_iterator ite2 = follow.begin(); ite2 != follow.end(); ++ite2) {
			if (parsingTable_->find(left, *ite2) == parsingTable_->end()) {
				parsingTable_->at(left, *ite2) = std::make_pair(GrammarSymbol::synch, nullptr);
			}
		}
	}
}

void LLParser::GetFirstSet(GrammarSymbolSet* answer, Condinate::iterator first, Condinate::iterator last) {
	if (first == last) {
		answer->insert(GrammarSymbol::epsilon);
		return;
	}

	for (; first != last; ++first) {
		if (first->SymbolType() == GrammarSymbolTerminal) {
			answer->insert(*first);

			if (*first != GrammarSymbol::epsilon) {
				break;
			}
		}
		else {
			Assert(firstSetContainer_.find(*first) != firstSetContainer_.end(), "logic error");
			GrammarSymbolSet& firstSet = firstSetContainer_[*first];
			answer->insert(firstSet.begin(), firstSet.end());
		
			if (firstSet.find(GrammarSymbol::epsilon) == firstSet.end()) {
				break;
			}
		}
	}
}

Grammar* LLParser::FindGrammar(const GrammarSymbol& left) {
	Grammar* g = nullptr;
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		if ((*ite)->GetLeft() == left) {
			g = *ite;
			break;
		}
	}

	return g;
}

GrammarSymbol LLParser::FindSymbol(const ScannerToken& token) {
	GrammarSymbol answer;
	if (token.tokenType == ScannerTokenEndOfFile) {
		answer = GrammarSymbol::zero;
	}
	else if (token.tokenType == ScannerTokenNumber || token.tokenType == ScannerTokenID) {
		BuildinSymbolContainer::iterator pos = buildinSymbols_.find(token.text);
		if (pos != buildinSymbols_.end()) {
			answer = pos->second;
		}
		else {
			answer = (token.tokenType == ScannerTokenNumber) ? GrammarSymbol::number : GrammarSymbol::identifier;
		}
	}
	else if (token.tokenType == ScannerTokenString) {
		answer = GrammarSymbol::string;
	}
	else {
		GrammarSymbolContainer::const_iterator ite = symbolContainer_.find(token.text);
		if (ite != symbolContainer_.end()) {
			answer = ite->second;
		}
	}

	return answer;
}
/*
bool LLParser::ParseFile(SyntaxTree* tree, FileScanner* fileScanner) {
	ScannerToken token;
	TokenPosition tokenPosition = { 0 };
	if (!fileScanner->GetToken(&token, &tokenPosition)) {
		Debug::LogError("failed to read token");
		return false;
	}

	std::string error = "invalid syntax";

	typedef std::pair<GrammarSymbol, SyntaxNode*> StackItem;
	std::stack<StackItem> s;
	GrammarSymbol symbol = grammars_.front()->GetLeft();
	SyntaxNode* root = nullptr;
	root = tree->AddNode(root, symbol.ToString());

	s.push(std::make_pair(symbol, root));

	for (; !s.empty();) {
		StackItem& item = s.top();
		symbol = item.first;
		root = item.second;

		if (symbol.SymbolType() == GrammarSymbolTerminal && symbol.Match(token.text)) {
			s.pop();

			if (symbol != GrammarSymbol::epsilon && !fileScanner->GetToken(&token, &tokenPosition)) {
				Debug::LogError("failed to read token");
				return false;
			}

			continue;
		}

		if (symbol.SymbolType() == GrammarSymbolNonterminal) {
			GrammarSymbol tokenSymbol = FindSymbol(token);
			if (!tokenSymbol) {
				error = std::string("unexpected token ") + token.text + " at " + tokenPosition.ToString();
				break;
			}

			ParsingTable::iterator pos = parsingTable_->find(symbol, tokenSymbol);
			if (pos == parsingTable_->end()) {
				if (!fileScanner->GetToken(&token, &tokenPosition)) {
					Debug::LogError("failed to read token");
					return false;
				}
			}
			else {
				Condinate* cond = pos->second.second;
				s.pop();

				if (cond != nullptr) {
					for (Condinate::reverse_iterator rite = cond->rbegin(); rite != cond->rend(); ++rite) {
						s.push(std::make_pair(*rite, tree->AddNode(root, rite->ToString())));
					}
				}
			}

			continue;
		}

		error = std::string("unexpected token ") + token.text + " at " + tokenPosition.ToString();
		break;
	}

	if (s.empty() && token.tokenType == ScannerTokenEndOfFile) {
		return true;
	}

	Debug::LogError(error);
	return false;
}
*/

bool LLParser::ParseFile(SyntaxTree* tree, FileScanner* fileScanner) {
	ScannerToken token;
	TokenPosition tokenPosition = { 0 };
	if (!fileScanner->GetToken(&token, &tokenPosition)) {
		Debug::LogError("failed to read token");
		return false;
	}

	std::string error = "invalid syntax";

	typedef std::pair<GrammarSymbol, SyntaxNode*> StackItem;
	std::stack<StackItem> s;
	GrammarSymbol symbol = grammars_.front()->GetLeft();
	SyntaxNode* root = nullptr;
	root = tree->AddNode(root, symbol.ToString());

	s.push(std::make_pair(symbol, root));

	for (; !s.empty();) {
		StackItem& item = s.top();
		symbol = item.first;
		root = item.second;

		if (symbol.SymbolType() == GrammarSymbolTerminal && symbol.Match(token.text)) {
			s.pop();

			if (symbol != GrammarSymbol::epsilon && !fileScanner->GetToken(&token, &tokenPosition)) {
				Debug::LogError("failed to read token");
				return false;
			}

			continue;
		}

		if (symbol.SymbolType() == GrammarSymbolNonterminal) {
			GrammarSymbol tokenSymbol = FindSymbol(token);
			if (!tokenSymbol) {
				error = std::string("invalid token ") + token.text + " at " + tokenPosition.ToString();
				break;
			}

			ParsingTable::iterator pos = parsingTable_->find(symbol, tokenSymbol);
			if (pos != parsingTable_->end()) {
				s.pop();

				Condinate* cond = pos->second.second;
				for (Condinate::reverse_iterator rite = cond->rbegin(); rite != cond->rend(); ++rite) {
					s.push(std::make_pair(*rite, tree->AddNode(root, rite->ToString())));
				}

				continue;
			}
		}

		error = std::string("unexpected token ") + token.text + " at " + tokenPosition.ToString();
		break;
	}

	if (s.empty() && token.tokenType == ScannerTokenEndOfFile) {
		return true;
	}

	Debug::LogError(error);
	return false;
}

bool LLParser::IsTerminal(const char* token) {
	return *token != '$';
}
