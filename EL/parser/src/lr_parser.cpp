#include "debug.h"
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
	GrammarSymbol a = nullptr;

	std::vector<int> stateStack(1, 0);
	std::vector<void*> valueStack(1, nullptr);
	std::vector<GrammarSymbol> symbolStack(1, GrammarSymbol::zero);

	LRAction action = { LRActionShift };
	int nextState = 0;
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
			nextState = action.actionParameter;
			Debug::Log(">> Shift symbol `" + a.ToString() + "`. Goto state " + std::to_string(nextState) + ".");

			stateStack.push_back(nextState);
			symbolStack.push_back(a);
			valueStack.push_back(addr);
		}
		else if (action.actionType == LRActionReduce) {
			Grammar* g = nullptr;
			const Condinate* cond = grammars_.GetTargetCondinate(action.actionParameter, &g);

			int length = cond->symbols.front() == GrammarSymbol::epsilon ? 0 : cond->symbols.size();

			std::string log = ">> Reduce `" + Utility::Concat(symbolStack.end() - length, symbolStack.end()) + "` to `" + g->GetLhs().ToString() + "`. ";

			void* newValue = (cond->action != nullptr) ? cond->action->Invoke(valueStack) : nullptr;

			stateStack.erase(stateStack.end() - length, stateStack.end());
			symbolStack.erase(symbolStack.end() - length, symbolStack.end());
			valueStack.erase(valueStack.end() - length, valueStack.end());

			nextState = lrTable_->GetNextGotoState(stateStack.back(), g->GetLhs());
			Debug::Log(log + "Goto state " + std::to_string(nextState) + ".");

			if (nextState < 0) {
				Debug::LogError("empty goto item(" + std::to_string(stateStack.back()) + ", " + g->GetLhs().ToString() + ")");
				return false;
			}

			stateStack.push_back(nextState);
			symbolStack.push_back(g->GetLhs());
			valueStack.push_back(newValue);
		}

	} while (action.actionType != LRActionAccept);

	tree->SetRoot((SyntaxNode*)valueStack.back());
	Debug::Log(Utility::Heading("Accept"));

	return true;
}

bool LRParser::ParseGrammars() {
	LRSetupParameter parameter = {
		&grammars_,
		&terminalSymbols_, &nonterminalSymbols_, 
		&firstSetContainer_, &followSetContainer_
	};

	return lrTable_->Create(parameter);
}

void LRParser::Clear() {
	Parser::Clear();
}

std::string LRParser::ToString() const {
	std::ostringstream oss;

	oss << Parser::ToString();
	oss << "\n\n";

	oss << firstSetContainer_.ToString();
	oss << "\n\n";
	oss << lrTable_->ToString();

	return oss.str();
}
