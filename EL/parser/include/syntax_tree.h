#pragma once
#include <string>
#include <sstream>
#include "constants.h"

class SyntaxNode {
private:
	friend class SyntaxTree;

	SyntaxNode(const std::string& name);
	~SyntaxNode();

	void AddChild(SyntaxNode* child);
	SyntaxNode* GetChild(int index);
	int ChildCount() const;

	const std::string& ToString() const;

private:
	int index_;
	std::string name_;
	SyntaxNode* children_[MAX_SYNTAX_NODE_CHILDREN];
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
	SyntaxTree(const SyntaxTree&);
	SyntaxTree& operator = (const SyntaxTree&);

	void SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* current) const;
	void DeleteTreeNode(SyntaxNode* node);

	typedef void (SyntaxTree::*TreeWalkCallback)(SyntaxNode* node);
	void PreorderTreeWalk(TreeWalkCallback callback);

private:
	SyntaxNode* root_;
};
