#include "debug.h"
#include "token.h"
#include "table.h"
#include "action.h"
#include "define.h"
#include "scanner.h"
#include "utilities.h"
#include "syntax_tree.h"

void Action::SetArgument(const Argument& argument) {
	argument_ = argument;
}

Action::~Action() {

}

bool Action::ParseParameters(TextScanner& scanner, Argument& argument) {
	int count = MAX_PARSER_FUNCTION_PARAMTERS;
	int parameters[MAX_PARSER_FUNCTION_PARAMTERS];
	if (!SplitParameters(parameters, count, scanner)) {
		return false;
	}

	argument.parameters.insert(argument.parameters.end(), parameters, parameters + count);
	return true;
}

bool Action::SplitParameters(int* parameters, int& count, TextScanner& scanner) {
	char token[MAX_TOKEN_CHARACTERS];
	ScannerTokenType tokenType = ScannerTokenEndOfFile, expectedTokenType = ScannerTokenIdentifier;

	int index = 0;
	for (; (tokenType = scanner.GetToken(token)) != ScannerTokenRightParenthesis;) {
		if (tokenType == ScannerTokenLeftParenthesis) {
			continue;
		}

		Assert(tokenType == expectedTokenType, std::string("invalid token type ") + std::to_string(tokenType));
		if (tokenType == ScannerTokenComma) {
			expectedTokenType = ScannerTokenIdentifier;
		}
		else {
			int integer = 0;
			bool valid = (*token == '$') && Utility::ParseInteger(token + 1, &integer);
			Assert(valid, std::string("invalid parameter format ") + token);
			Assert(index < count, "buffer too small");

			parameters[index++] = integer;
			expectedTokenType = ScannerTokenComma;
		}
	}

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenEndOfFile, "invalid tailing characters");

	count = index;

	return true;
}

std::string ActionConstant::ToString() const {
	return std::string("constant($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionConstant::Invoke(const std::vector<void*>& container) {
	Constant* constant = (Constant*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeConstant, constant->ToString());
	ans->SetConstantAddress(constant);
	return ans;
}

std::string ActionLiteral::ToString() const {
	return std::string("literal($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionLiteral::Invoke(const std::vector<void*>& container) {
	Literal* literal = (Literal*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeLiteral, literal->ToString());
	ans->SetLiteralAddress(literal);
	return ans;
}

std::string ActionSymbol::ToString() const {
	return std::string("symbol($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionSymbol::Invoke(const std::vector<void*>& container) {
	Sym* sym = (Sym*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeSymbol, sym->ToString());
	ans->SetSymbolAddress(sym);
	return ans;
}

std::string ActionIndex::ToString() const {
	return std::string("$") + std::to_string(argument_.parameters.front());
}

SyntaxNode* ActionIndex::Invoke(const std::vector<void*>& container) {
	return (SyntaxNode*)container[container.size() - argument_.parameters.front()];
}

std::string ActionMake::ToString() const {
	const char* sep = "";
	std::ostringstream oss;
	
	oss << "make(";

	for (std::vector<int>::const_iterator ite = argument_.parameters.begin();
		ite != argument_.parameters.end(); ++ite) {
		oss << sep;
		oss << '$' << std::to_string(*ite);
		sep = ", ";
	}

	oss << ")";

	return oss.str();
}

SyntaxNode* ActionMake::Invoke(const std::vector<void*>& container) {
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeOperation, argument_.text);
	SyntaxNode** nodes = new SyntaxNode*[argument_.parameters.size()];
	for (int i = 0; i < (int)argument_.parameters.size(); ++i) {
		if (argument_.parameters[i] == 0) {
			nodes[i] = nullptr;
		}
		else {
			nodes[i] = (SyntaxNode*)container[container.size() - argument_.parameters[i]];
		}
	}

	ans->AddChildren(nodes, argument_.parameters.size());
	delete[] nodes;

	return ans;
}

bool ActionMake::ParseParameters(TextScanner& scanner, Argument& argument) {
	char token[MAX_TOKEN_CHARACTERS];
	ScannerTokenType tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenLeftParenthesis, "invalid parameter");

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenString && strlen(token) > 0, "invalid action name");
	argument.text = token;

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenComma, "invalid parameter");

	return Action::ParseParameters(scanner, argument);
}

Action* ActionParser::Parse(const std::string& cmd) {
	if (cmd.empty()) {
		return nullptr;
	}

	return CreateAction(cmd);
}

void ActionParser::Destroy(Action* action) {
	delete action;
}

Action* ActionParser::CreateAction(const std::string& cmd) {
	TextScanner scanner;
	scanner.SetText(cmd.c_str());

	char token[MAX_TOKEN_CHARACTERS];
	ScannerTokenType tokenType = scanner.GetToken(token);

	// TODO: 
	if (tokenType == ScannerTokenEndOfFile) {
		return nullptr;
	}

	Assert(tokenType == ScannerTokenIdentifier && strcmp(token, "$$") == 0, std::string("invalid left hand side operand: ") + token);

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenAssign, "missing '='");

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenIdentifier, "invalid command");

	Action* action = nullptr;
	Argument argument;

	if (IsOperand(token)) {
		int integer = 0;
		if (!Utility::ParseInteger(token + 1, &integer)) {
			Assert(false, std::string("invalid right operand ") + token);
		}
		else {
			action = new ActionIndex();
			argument.text = "identity";
			argument.parameters.push_back(integer);
		}
	}
	else {
		if (strcmp(token, "make") == 0) {
			action = new ActionMake();
			argument.text = "make";
		}
		else if (strcmp(token, "constant") == 0) {
			action = new ActionConstant();
			argument.text = "constant";
		}
		else if (strcmp(token, "symbol") == 0) {
			action = new ActionSymbol();
			argument.text = "symbol";
		}
		else if (strcmp(token, "literal") == 0) {
			action = new ActionLiteral();
			argument.text = "literal";
		}

		if (action != nullptr && !action->ParseParameters(scanner, argument)) {
			delete action;
			action = nullptr;
		}
	}

	Assert(action != nullptr, "invalid action " + token);
	action->SetArgument(argument);

	return action;
}

bool ActionParser::IsOperand(const char* text) {
	if (strlen(text) < 2) {
		return false;
	}

	if (*text != '$') {
		return false;
	}

	++text;
	if (*text != '$' && !Utility::ParseInteger(text)) {
		return false;
	}

	return true;
}
