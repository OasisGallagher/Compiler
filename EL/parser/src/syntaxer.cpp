#include "debug.h"
#include "parser.h"
#include "action.h"
#include "scanner.h"
#include "syntaxer.h"
#include "lr_table.h"
#include "serializer.h"
#include "syntax_tree.h"

struct SyntaxerStack {
	std::vector<int> states;
	std::vector<void*> values;
	std::vector<GrammarSymbol> symbols;

	void push(int state, void* value, const GrammarSymbol& symbol);
	void pop(int count);
	void clear();
};

Syntaxer::Syntaxer() {
	symTable_ = new SymTable;
	stack_ = new SyntaxerStack;
	literalTable_ = new LiteralTable;
	constantTable_ = new ConstantTable;
}

Syntaxer::~Syntaxer() {
	delete stack_;
	delete symTable_;
	delete literalTable_;
	delete constantTable_;
}

void Syntaxer::Setup(const SyntaxerSetupParameter& p) {
	p_ = p;
}

bool Syntaxer::Load(std::ifstream& file) {
	return Serializer::LoadSyntaxer(file, p_);
}

bool Syntaxer::Save(std::ofstream& file) {
	return Serializer::SaveSyntaxer(file, p_);
}

bool Syntaxer::ParseSyntax(SyntaxTree* tree, FileScanner* fileScanner) {
	SyntaxNode* root = nullptr;
	if (!CreateSyntaxTree(root, fileScanner)) {
		return false;
	}

	tree->SetRoot(root);
	Debug::Log("\n" + Utility::Heading("Accept"));
	return true;

}

std::string Syntaxer::ToString() const {
	return p_.lrTable.ToString(p_.env->grammars);
}

int Syntaxer::Reduce(int cpos) {
	Grammar* g = nullptr;
	const Condinate* cond = p_.env->grammars.GetTargetCondinate(cpos, &g);

	int length = cond->symbols.front() == NativeSymbols::epsilon ? 0 : cond->symbols.size();

	std::string log = ">> [R] `" + Utility::Concat(stack_->symbols.end() - length, stack_->symbols.end()) + "` to `" + g->GetLhs().ToString() + "`. ";

	void* newValue = (cond->action != nullptr) ? cond->action->Invoke(stack_->values) : nullptr;

	stack_->pop(length);

	int nextState = p_.lrTable.GetGoto(stack_->states.back(), g->GetLhs());
	Debug::Log(log + "Goto state " + std::to_string(nextState) + ".");

	if (nextState < 0) {
		Debug::LogError("empty goto item(" + std::to_string(stack_->states.back()) + ", " + g->GetLhs().ToString() + ")");
		return nextState;
	}

	stack_->push(nextState, newValue, g->GetLhs());
	return nextState;
}

bool Syntaxer::Error(const GrammarSymbol& symbol, const TokenPosition& position) {
	Debug::LogError("unexpected symbol " + symbol.ToString() + " at " + position.ToString());
	return false;
}

void Syntaxer::Shift(int state, void* addr, const GrammarSymbol& symbol) {
	Debug::Log(">> [S] `" + symbol.ToString() + "`. Goto state " + std::to_string(state) + ".");
	stack_->push(state, addr, symbol);
}

bool Syntaxer::CreateSyntaxTree(SyntaxNode*& root, FileScanner* fileScanner) {
	TokenPosition position = { 0 };

	stack_->push(0, nullptr, NativeSymbols::zero);
	LRAction action = { LRActionShift };

	void* addr = nullptr;
	GrammarSymbol symbol = nullptr;

	do {
		if (action.actionType == LRActionShift && !(symbol = ParseNextSymbol(position, addr, fileScanner))) {
			break;
		}

		action = p_.lrTable.GetAction(stack_->states.back(), symbol);

		if (action.actionType == LRActionError && !Error(symbol, position)) {
			break;
		}

		if (action.actionType == LRActionShift) {
			Shift(action.actionParameter, addr, symbol);
		}
		else if (action.actionType == LRActionReduce) {
			if (!Reduce(action.actionParameter)) {
				break;
			}
		}

	} while (action.actionType != LRActionAccept);

	if (action.actionType == LRActionAccept) {
		root = (SyntaxNode*)stack_->values.back();
	}

	stack_->clear();
	return action.actionType == LRActionAccept;
}

GrammarSymbol Syntaxer::FindSymbol(const ScannerToken& token, void*& addr) {
	addr = nullptr;

	GrammarSymbol answer = NativeSymbols::null;
	if (token.tokenType == ScannerTokenEndOfFile) {
		answer = NativeSymbols::zero;
	}
	else if (token.tokenType == ScannerTokenNumber) {
		answer = NativeSymbols::number;
		addr = constantTable_->Add(token.text);
	}
	else if (token.tokenType == ScannerTokenString) {
		answer = NativeSymbols::string;
		addr = literalTable_->Add(token.text);
	}
	else {
		GrammarSymbolContainer::const_iterator pos = p_.env->terminalSymbols.find(token.text);
		if (pos != p_.env->terminalSymbols.end()) {
			answer = pos->second;
		}
		else {
			answer = NativeSymbols::identifier;
			addr = symTable_->Add(token.text);
		}
	}

	Assert(answer != NativeSymbols::null, std::string("can not find symbol") + token.text);

	return answer;
}

GrammarSymbol Syntaxer::ParseNextSymbol(TokenPosition& position, void*& addr, FileScanner* fileScanner) {
	ScannerToken token;
	GrammarSymbol answer = NativeSymbols::null;

	if (fileScanner->GetToken(&token, &position)) {
		answer = FindSymbol(token, addr);
	}

	if (!answer) {
		Debug::LogError("invalid token at " + position.ToString());
	}

	return answer;
}

void SyntaxerStack::push(int state, void* value, const GrammarSymbol& symbol) {
	states.push_back(state);
	values.push_back(value);
	symbols.push_back(symbol);
}

void SyntaxerStack::pop(int count) {
	states.erase(states.end() - count, states.end());
	values.erase(values.end() - count, values.end());
	symbols.erase(symbols.end() - count, symbols.end());
}

void SyntaxerStack::clear() {
	states.clear();
	values.clear();
	symbols.clear();
}
