#include "action.h"
#include "matrix.h"
#include "scanner.h"
#include "lr_parser.h"
#include "syntax_tree.h"
#include "table_printer.h"

LRParser::LRParser() {
	lr0_ = new LR0();
}

LRParser::~LRParser() {
	delete lr0_;
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

		action = actionTable_.at(stateStack.back(), a);
		
		if (action.actionType == LRActionError) {
			Debug::LogError("unexpected symbol " + a.ToString() + " at " + position.ToString());
			return false;
		}

		if (action.actionType == LRActionShift) {
			if (!gotoTable_.get(stateStack.back(), a, nextState)) {
				Debug::LogError("empty goto item(" + std::to_string(stateStack.back()) + ", " + a.ToString() + ")");
				return false;
			}

			stateStack.push_back(nextState);
			symbolStack.push_back(a);
			valueStack.push_back(addr);
		}
		else if (action.actionType == LRActionReduce) {
			Grammar* g = nullptr;
			Condinate* cond = LR0::GetTargetCondinate(grammars_, action.actionParameter, &g);

			int length = cond->symbols.size();
			Debug::Log(std::to_string(++reduceCount) + "\tReduce `" + Utility::Concat(symbolStack.end() - length, symbolStack.end()) + "` to `" + g->GetLhs().ToString() + "`");

			stateStack.erase(stateStack.end() - length, stateStack.end());
			symbolStack.erase(symbolStack.end() - length, symbolStack.end());
			valueStack.erase(valueStack.end() - length, valueStack.end());

			if (!gotoTable_.get(stateStack.back(), g->GetLhs(), nextState)) {
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
	lr0_->Parse(&grammars_, &terminalSymbols_, &nonterminalSymbols_);
	return true;
}

void LRParser::Clear() {
	Parser::Clear();
}

std::string LRParser::ToString() const {
	std::ostringstream oss;
	oss << Parser::ToString();

	oss << "\n\n";

	oss << lr0_->ToString();

	return oss.str();
}
