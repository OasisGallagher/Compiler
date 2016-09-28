#include "debug.h"
#include "token.h"
#include "action.h"
#include "stack2.h"
#include "scanner.h"
#include "constants.h"
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
	for (; (tokenType = scanner.GetToken(token)) != ScannerTokenSign || strcmp(token, ")") != 0;) {
		if (tokenType == ScannerTokenSign && strcmp(token, "(") == 0) {
			continue;
		}

		Assert(tokenType == expectedTokenType, std::string("invalid token type ") + std::to_string(tokenType));
		if (tokenType == ScannerTokenSign) {
			Assert(strcmp(token, ",") == 0, "invalid seperator");
			expectedTokenType = ScannerTokenIdentifier;
		}
		else {
			Assert(strlen(token) == 2 && token[0] == '$' && isdigit(token[1]), std::string("invalid parameter format ") + token);
			Assert(index < count, "buffer too small");

			parameters[index++] = token[1] - '0';
			expectedTokenType = ScannerTokenSign;
		}
	}

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenEndOfFile, "invalid tailing characters");

	count = index;

	return true;
}

std::string ActionConstant::ToString() const {
	return "";
}

SyntaxNode* ActionConstant::Invoke(const std::vector<void*>& container) {
	return new SyntaxNode(SyntaxNodeConstant, "constant");
}

std::string ActionSymbol::ToString() const {
	return "";
}

SyntaxNode* ActionSymbol::Invoke(const std::vector<void*>& container) {
	return new SyntaxNode(SyntaxNodeSymbol, "identifier");
}

std::string ActionIdentity::ToString() const {
	return "";
}

SyntaxNode* ActionIdentity::Invoke(const std::vector<void*>& container) {
	return (SyntaxNode*)container[container.size() - argument_.parameters.front()];
}

bool ActionIdentity::ParseParameters(TextScanner& scanner, Argument& argument) {
	return true;
}

std::string ActionMake::ToString() const {
	return "";
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
	Assert(tokenType == ScannerTokenSign && strcmp(token, "(") == 0, "invalid parameter");

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenString && strlen(token) > 0, "invalid action name");
	argument.text = token;

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenSign && strcmp(token, ",") == 0, "invalid parameter");

	return Action::ParseParameters(scanner, argument);
}

ActionParser::~ActionParser() {
	for (std::vector<Action*>::iterator ite = actions_.begin(); ite != actions_.end(); ++ite) {
		delete *ite;
	}
}

Action* ActionParser::Parse(const std::string& cmd) {
	Action* ans = CreateAction(cmd);
	if (ans != nullptr) {
		actions_.push_back(ans);
	}

	return ans;
}

Action* ActionParser::CreateAction(const std::string& cmd) {
	TextScanner scanner;
	scanner.SetText(cmd.c_str());

	char token[MAX_TOKEN_CHARACTERS];
	ScannerTokenType tokenType = scanner.GetToken(token);

	Assert(tokenType == ScannerTokenIdentifier && strcmp(token, "$$") == 0, std::string("invalid left hand side operand: ") + token);

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenSign && strcmp(token, "=") == 0, "missing '='");

	tokenType = scanner.GetToken(token);
	Assert(tokenType == ScannerTokenIdentifier, "invalid command");

	Action* action = nullptr;
	Argument argument;

	if (IsOperand(token)) {
		action = new ActionIdentity();
		argument.text = "identity";
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

		if (action != nullptr && !action->ParseParameters(scanner, argument)) {
			delete action;
		}
	}

	Assert(action != nullptr, "invalid command " + token);
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
	if (*text != '$' && !isdigit(*text)) {
		return false;
	}

	++text;
	return *text == 0;
}
