#pragma once
#include <string>
#include <sstream>
#include "constants.h"

class SyntaxNode {
private:
	SyntaxNode(const std::string& name);
	~SyntaxNode();

	void AddChild(SyntaxNode* child);
	SyntaxNode* GetChild(int index);
	int ChildCount() const;

	const std::string& ToString() const;

	friend class SyntaxTree;
private:
	int index_;
	std::string name_;
	SyntaxNode* children_[Constants::kMaxSyntaxNodeChildren];
	SyntaxNode* sibling_;
};

class SyntaxTree {
public:
	SyntaxTree();
	~SyntaxTree();

	SyntaxNode* AddNode(SyntaxNode* parent, const std::string& name);

public:
	std::string ToString() const;

private:
	void SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* current, bool tail) const;

private:
	SyntaxNode* root_;
};
