#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "constants.h"

enum SyntaxNodeType {
	SyntaxNodeConstant,
	SyntaxNodeSymbol,
	SyntaxNodeOperation,
};

class SyntaxNode {
public:
	SyntaxNode(SyntaxNodeType type, const std::string& text);
	~SyntaxNode();

public:
	void AddChildren(SyntaxNode** buffer, int count);

	SyntaxNode* GetChild(int index);
	int GetChildCount() const;

	void SetConstantAddress(void* addr);
	void SetSymbolAddress(void* addr);

	const std::string& ToString() const;

private:
	std::string text_;
	SyntaxNodeType type_;

	union {
		// 子节点, 第一个元素表示子节点个数.
		SyntaxNode** children;
		void* constant;
		void* symbol;
	} value_;
};

/*
class SyntaxTree {
public:
	SyntaxTree();
	~SyntaxTree();

public:
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
};*/
