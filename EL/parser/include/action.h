#pragma once

#include <vector>

class SyntaxNode;
class TextScanner;

struct Argument {
	std::string text;
	std::vector<int> parameters;
};

class Action {
public:
	virtual ~Action();

public:
	void SetArgument(const Argument& argument);

public:
	virtual std::string ToString() const = 0;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container) = 0;
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);

private:
	bool SplitParameters(int* parameters, int& count, TextScanner& scanner);

protected:
	Argument argument_;
};

class ActionConstant : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionLiteral : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionSymbol : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionIndex : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionMake : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);
};

class ActionParser {
public:
	static Action* Parse(const std::string& cmd);
	static void Destroy(Action* action);

private:
	static Action* CreateAction(const std::string& cmd);
	static bool IsOperand(const char* text);
};
