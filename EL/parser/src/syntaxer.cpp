#include "debug.h"
#include "table.h"
#include "parser.h"
#include "action.h"
#include "grammar.h"
#include "scanner.h"
#include "lr_impl.h"
#include "syntaxer.h"
#include "lr_table.h"
#include "serializer.h"
#include "syntax_tree.h"

Syntaxer::Syntaxer() {
	symTable_ = new SymTable();
	literalTable_ = new LiteralTable();
	constantTable_ = new ConstantTable();
}

Syntaxer::~Syntaxer() {
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
	SyntaxNode* root = CreateSyntaxTree(fileScanner);
	if (root != nullptr) {
		tree->SetRoot(root);
		Debug::Log("\n" + Utility::Heading("Accept"));
		return true;
	}

	return false;
}

std::string Syntaxer::ToString() const {
	return p_.lrTable.ToString();
}

SyntaxNode* Syntaxer::CreateSyntaxTree(FileScanner* fileScanner) {
	TokenPosition position = { 0 };
	GrammarSymbol a = nullptr;

	std::vector<int> stateStack(1, 0);
	std::vector<void*> valueStack(1, nullptr);
	std::vector<GrammarSymbol> symbolStack(1, NativeSymbols::zero);

	LRAction action = { LRActionShift };
	int nextState = 0;
	void* addr = nullptr;

	do {
		if (action.actionType == LRActionShift && !(a = ParseNextSymbol(position, addr, fileScanner))) {
			return nullptr;
		}

		action = p_.lrTable.GetAction(stateStack.back(), a);

		if (action.actionType == LRActionError) {
			Debug::LogError("unexpected symbol " + a.ToString() + " at " + position.ToString());
			return nullptr;
		}

		if (action.actionType == LRActionShift) {
			nextState = action.actionParameter;
			Debug::Log(">> Shift\t`" + a.ToString() + "`. Goto state " + std::to_string(nextState) + ".");

			stateStack.push_back(nextState);
			symbolStack.push_back(a);
			valueStack.push_back(addr);
		}
		else if (action.actionType == LRActionReduce) {
			Grammar* g = nullptr;
			const Condinate* cond = p_.env->grammars.GetTargetCondinate(action.actionParameter, &g);

			int length = cond->symbols.front() == NativeSymbols::epsilon ? 0 : cond->symbols.size();

			std::string log = ">> Reduce\t`" + Utility::Concat(symbolStack.end() - length, symbolStack.end()) + "` to `" + g->GetLhs().ToString() + "`. ";

			void* newValue = (cond->action != nullptr) ? cond->action->Invoke(valueStack) : nullptr;

			stateStack.erase(stateStack.end() - length, stateStack.end());
			symbolStack.erase(symbolStack.end() - length, symbolStack.end());
			valueStack.erase(valueStack.end() - length, valueStack.end());

			nextState = p_.lrTable.GetGoto(stateStack.back(), g->GetLhs());
			Debug::Log(log + "Goto state " + std::to_string(nextState) + ".");

			if (nextState < 0) {
				Debug::LogError("empty goto item(" + std::to_string(stateStack.back()) + ", " + g->GetLhs().ToString() + ")");
				return nullptr;
			}

			stateStack.push_back(nextState);
			symbolStack.push_back(g->GetLhs());
			valueStack.push_back(newValue);
		}

	} while (action.actionType != LRActionAccept);

	return (SyntaxNode*)valueStack.back();
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
	else if (Utility::IsTerminal(token.text)) {
		GrammarSymbolContainer::const_iterator pos = p_.env->terminalSymbols.find(token.text);
		if (pos != p_.env->terminalSymbols.end()) {
			answer = pos->second;
		}
		else {
			answer = NativeSymbols::identifier;
			addr = symTable_->Add(token.text);
		}
	}
	else {
		GrammarSymbolContainer::const_iterator ite = p_.env->nonterminalSymbols.find(token.text);
		if (ite != p_.env->nonterminalSymbols.end()) {
			answer = ite->second;
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
