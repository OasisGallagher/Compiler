#pragma once
#include <string>
#include <sstream>
#include "constants.h"
#include "grammar_symbol.h"

class SyntaxNode {
public:
	SyntaxNode();
	~SyntaxNode();

public:
	void AddChild(SyntaxNode* child);
	SyntaxNode* GetChild(int index);
	int ChildCount() const;

	std::string ToString() const;

	friend class SyntaxTree;
private:
	int index_;
	SyntaxNode* children_[Constants::kMaxSyntaxNodeChildren];
	SyntaxNode* sibling_;
};

class SyntaxTree {
public:
	SyntaxTree();
	~SyntaxTree();

	SyntaxNode* AddNode(SyntaxNode* parent, const GrammarSymbol& symbol);

public:
	std::string ToString() const;

private:
	void SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* current, bool tail) const;

private:
	SyntaxNode* root_;
};
