#include "action.h"
#include "matrix.h"
#include "scanner.h"
#include "lr_table.h"
#include "lr_parser.h"
#include "syntax_tree.h"

LRParser::LRParser() {
	lrTable_ = new LRTable();
}

LRParser::~LRParser() {
	delete lrTable_;
}

GrammarSymbol LRParser::ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner) {
	ScannerToken token;
	GrammarSymbol answer = GrammarSymbol::null;
	
	if (fileScanner->GetToken(&token, &position)) {
		answer = FindSymbol(token, addr);
	}

	if (!answer) {
		Debug::LogError("invalid token at " + position.ToString());
	}

	return answer;
}

bool LRParser::ParseFile(SyntaxTree* tree, FileScanner* fileScanner) {
	TokenPosition position = { 0 };
	GrammarSymbol a;

	std::vector<int> stateStack(1, 0);
	std::vector<void*> valueStack(1, nullptr);
	std::vector<GrammarSymbol> symbolStack(1, GrammarSymbol::zero);

	LRAction action = { LRActionError };
	int nextState = 0, reduceCount = 0;
	void* addr = nullptr;

	do {
		if (action.actionType == LRActionShift && !(a = ParseNextSymbol(position, addr, fileScanner))) {
			return false;
		}

		action = lrTable_->GetAction(stateStack.back(), a);
		
		if (action.actionType == LRActionError) {
			Debug::LogError("unexpected symbol " + a.ToString() + " at " + position.ToString());
			return false;
		}

		if (action.actionType == LRActionShift) {
			nextState = lrTable_->GetNextState(stateStack.back(), a);
			if (nextState < 0) {
				Debug::LogError("empty goto item(" + std::to_string(stateStack.back()) + ", " + a.ToString() + ")");
				return false;
			}

			stateStack.push_back(nextState);
			symbolStack.push_back(a);
			valueStack.push_back(addr);
		}
		else if (action.actionType == LRActionReduce) {
			Grammar* g = nullptr;
			const Condinate* cond = grammars_.GetTargetCondinate(action.actionParameter, &g);

			int length = cond->symbols.size();
			Debug::Log(std::to_string(++reduceCount) + "\tReduce `" + Utility::Concat(symbolStack.end() - length, symbolStack.end()) + "` to `" + g->GetLhs().ToString() + "`");

			stateStack.erase(stateStack.end() - length, stateStack.end());
			symbolStack.erase(symbolStack.end() - length, symbolStack.end());
			valueStack.erase(valueStack.end() - length, valueStack.end());

			nextState = lrTable_->GetNextState(stateStack.back(), g->GetLhs());
			if (nextState < 0) {
				Debug::LogError("empty goto item(" + std::to_string(stateStack.back()) + ", " + g->GetLhs().ToString() + ")");
				return false;
			}

			stateStack.push_back(nextState);
			symbolStack.push_back(g->GetLhs());
			valueStack.push_back(cond->action->Invoke(valueStack));
		}

	} while (action.actionType != LRActionAccept);

	tree->SetRoot((SyntaxNode*)valueStack.back());
	Debug::Log(Utility::Heading("Accept"));

	return true;
}

bool LRParser::ParseGrammars() {
	lrTable_->Create(&grammars_, &terminalSymbols_, &nonterminalSymbols_);
	return true;
}

void LRParser::Clear() {
	Parser::Clear();
}

std::string LRParser::ToString() const {
	std::ostringstream oss;
	oss << Parser::ToString();

	oss << "\n\n";

	oss << lrTable_->ToString();

	return oss.str();
}
