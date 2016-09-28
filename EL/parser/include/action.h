#pragma once

#include <vector>

template <class Ty>
class Stack;
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

class ActionSymbol : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionIdentity : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);
};

class ActionMake : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);
};

class ActionParser {
public:
	~ActionParser();

public:
	Action* Parse(const std::string& cmd);

private:
	Action* CreateAction(const std::string& cmd);
	bool IsOperand(const char* text);

private:
	std::vector<Action*> actions_;
};
