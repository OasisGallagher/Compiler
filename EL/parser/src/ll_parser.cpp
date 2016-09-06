#include <stack>
#include <sstream>
#include <algorithm>
#include <functional>

#include "token.h"
#include "matrix.h"
#include "scanner.h"
#include "ll_parser.h"
#include "syntax_tree.h"

template <class Ty>
void TraceStack(const std::string& token, const Ty& s) {
	std::ostringstream oss;
	const char* seperator = "";
	for (Ty copy = s; !copy.empty();) {
		oss << seperator << copy.top().first.ToString();
		copy.pop();
		seperator = " ";
	}

	std::string space;
	for (int i = (int)token.length(); i < 12; ++i) {
		space += ' ';
	}
	
	Debug::Log(token + space + " => " + oss.str());
}

class ParsingTable : public matrix<GrammarSymbol, GrammarSymbol, std::pair<GrammarSymbol, Condinate*>> {
public:
	std::string ToString() const;
};

std::string ParsingTable::ToString() const {
	std::ostringstream oss;
	const char* seperator = "";
	for (const_iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
		const key_type& key = ite->first;
		const value_type& value = ite->second;

		oss << seperator;
		seperator = "\n";

		oss.width(28);
		oss.setf(std::ios::left);
		oss << "[" + key.first.ToString() + ", " + key.second.ToString() + "]";
		oss << value.first.ToString();

		if (value.second != nullptr) {
			oss << " : ";
			oss << value.second->ToString();
		}
	}

	return oss.str();
}

LLParser::LLParser() {
	parsingTable_ = new ParsingTable();
}

LLParser::~LLParser() {
	delete parsingTable_;
}

bool LLParser::ParseGrammars() {
	RemoveLeftRecursion();
	LeftFactoring();

	CreateFirstSets();
	CreateFollowSets();

	return CreateParsingTable();
}

void LLParser::Clear() {
	Parser::Clear();
	parsingTable_->clear();
	firstSetContainer_.clear();
	followSetContainer_.clear();
}

std::string LLParser::ToString() const {
	std::ostringstream oss;
	oss << Parser::ToString();

	oss << "\n\n";

	oss << Utility::Heading(" First ") << "\n";
	oss << firstSetContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" Follow ") << "\n";
	oss << followSetContainer_.ToString();

	oss << "\n\n";

	oss << Utility::Heading(" ParsingTable ") << "\n";
	oss << parsingTable_->ToString();

	return oss.str();
}

void LLParser::RemoveLeftRecursion() {
	GrammarContainer newGrammars;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end();) {
		if (RemoveImmidiateLeftRecursion(*ite, &newGrammars)) {
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

bool LLParser::RemoveImmidiateLeftRecursion(Grammar* g, GrammarContainer* newGrammars) {
	const GrammarSymbol& lhs = g->GetLhs();
	const CondinateContainer& condinates = g->GetCondinates();

	CondinateContainer::const_iterator pos = condinates.begin();
	for (; pos != condinates.end(); ++pos) {
		if ((*pos)->front() != lhs) {
			break;
		}
	}

	if (pos == condinates.begin()) {
		return false;
	}

	Assert(pos != condinates.end(), "invalid production");
	Grammar* grammar = new Grammar(lhs);

	GrammarSymbol lhs2 = CreateSymbol(lhs.ToString() + "_2");
	
	Grammar* grammar2 = new Grammar(lhs2);
	Condinate cond;

	for (CondinateContainer::const_iterator ite = pos; ite != condinates.end(); ++ite) {
		cond.insert(cond.end(), (*ite)->begin(), (*ite)->end());
		cond.push_back(lhs2);

		grammar->AddCondinate(cond);
		cond.clear();
	}

	for (CondinateContainer::const_iterator ite = condinates.begin(); ite != pos; ++ite) {
		cond.insert(cond.end(), (*ite)->begin() + 1, (*ite)->end());
		cond.push_back(lhs2);

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
		Grammar* grammar = new Grammar(g->GetLhs());

		GrammarSymbol lhs2 = CreateSymbol(g->GetLhs().ToString() + "_" + std::to_string(++nsindex));

		Condinate cond;
		const CondinateContainer& oldCondinates = g->GetCondinates();
		Condinate::iterator first = oldCondinates[from]->begin();
		Condinate::iterator last = first;
		std::advance(last, length);
		cond.insert(cond.end(), first, last);

		cond.push_back(lhs2);
		grammar->AddCondinate(cond);

		for (int i = 0; i < from; ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		for (int i = to; i < (int)oldCondinates.size(); ++i) {
			grammar->AddCondinate(*oldCondinates[i]);
		}

		Grammar* grammar2 = new Grammar(lhs2);
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

void LLParser::CreateFirstSets() {
	for (; CreateFirstSetsOnePass();) {
	}
}

bool LLParser::CreateFirstSetsOnePass() {
	bool anySetModified = false;

	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const CondinateContainer& conds = g->GetCondinates();
		GrammarSymbolSet& firstSet = firstSetContainer_[g->GetLhs()];

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
	followSetContainer_[grammars_.front()->GetLhs()].insert(GrammarSymbol::zero);

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
					anySetModified = MergeNonEpsilonElements(followSetContainer_[symbol], followSetContainer_[g->GetLhs()]) || anySetModified;
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

void LLParser::InsertParsingTable(const GrammarSymbol& k1, const GrammarSymbol& k2, Condinate* c) {
	ParsingTable::iterator pos = parsingTable_->find(k1, k2);
	if (pos != parsingTable_->end()) {
		std::string slot = "[" + pos->first.first.ToString() + ", " + pos->first.second.ToString() + "]";
		std::string p1 = pos->second.first.ToString() + " : " + pos->second.second->ToString();
		std::string p2 = k1.ToString() + " : " + c->ToString();
		Debug::LogWarning("invalid LL(1) grammar at " + slot + "\n(1) " + p1 + "\n(2) " + p2);
	}
	else {
		parsingTable_->insert(k1, k2, std::make_pair(k1, c));
	}
}

bool LLParser::BuildParingTable(Grammar* g) {
	const GrammarSymbol& lhs = g->GetLhs();
	const CondinateContainer& conds = g->GetCondinates();
	GrammarSymbolSet firstSet;

	for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
		Condinate* c = *ite;
		GetFirstSet(&firstSet, c->begin(), c->end());
		for (GrammarSymbolSet::iterator ite2 = firstSet.begin(); ite2 != firstSet.end(); ++ite2) {
			InsertParsingTable(lhs, *ite2, c);
		}

		if (firstSet.find(GrammarSymbol::epsilon) != firstSet.end()) {
			const GrammarSymbolSet& follow = followSetContainer_[lhs];
			for (GrammarSymbolSet::const_iterator ite3 = follow.begin(); ite3 != follow.end(); ++ite3) {
				InsertParsingTable(lhs, *ite3, c);
			}
		}

		firstSet.clear();
	}

	return true;
}

void LLParser::AddAsyncSymbol() {
	/*
	for (GrammarContainer::iterator ite = grammars_.begin(); ite != grammars_.end(); ++ite) {
		Grammar* g = *ite;
		const GrammarSymbol& lhs = g->GetLhs();
		GrammarSymbolSetTable::const_iterator pos = followSetContainer_.find(lhs);
		if (pos == followSetContainer_.end()) {
			continue;
		}

		const GrammarSymbolSet& follow = pos->second;
		for (GrammarSymbolSet::const_iterator ite2 = follow.begin(); ite2 != follow.end(); ++ite2) {
			if (parsingTable_->find(lhs, *ite2) == parsingTable_->end()) {
				parsingTable_->at(lhs, *ite2) = std::make_pair(GrammarSymbol::synch, nullptr);
			}
		}
	}*/
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
	GrammarSymbol symbol = grammars_.front()->GetLhs();
	SyntaxNode* root = nullptr;
	root = tree->AddNode(root, symbol.ToString());

	s.push(std::make_pair(symbol, root));

	for (; !s.empty();) {
		StackItem& item = s.top();
		symbol = item.first;
		root = item.second;

		if (symbol.SymbolType() == GrammarSymbolTerminal && symbol.Match(token.text)) {
			s.pop();
			TraceStack("Match " + std::string(token.text), s);
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

				TraceStack("Push " + std::string(token.text), s);

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
