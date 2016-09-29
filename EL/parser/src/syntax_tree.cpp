#include <stack>
#include "debug.h"
#include "syntax_tree.h"

#define DEBUG_NODE_TYPE(current, expected)	\
	Assert(current == expected, std::string("invalid syntax node type: ") + std::to_string(current) + " != " + std::to_string(expected));

SyntaxNode::SyntaxNode(SyntaxNodeType type, const std::string& text)
	: text_(text), type_(type) {
	memset(&value_, 0, sizeof(value_));
}

SyntaxNode::~SyntaxNode() {
	if (type_ == SyntaxNodeOperation) {
		delete[] value_.children;
	}
}

SyntaxNodeType SyntaxNode::GetNodeType() const {
	return type_;
}

void SyntaxNode::AddChildren(SyntaxNode** buffer, int count) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);

	value_.children = new SyntaxNode*[count + 1];
	*value_.children = (SyntaxNode*)count;

	for (int i = 1; i <= count; ++i) {
		value_.children[i] = buffer[i - 1];
	}
}

SyntaxNode* SyntaxNode::GetChild(int index) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);
	Assert(index >= 0 && index < GetChildCount(), "index out of range");
	return value_.children[index + 1];
}

const SyntaxNode* SyntaxNode::GetChild(int index) const {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);
	Assert(index >= 0 && index < GetChildCount(), "index out of range");
	return value_.children[index + 1];
}

int SyntaxNode::GetChildCount() const {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);
	return (int)*value_.children;
}

void SyntaxNode::SetConstantAddress(Constant* addr) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeConstant);
	value_.constant = addr;
}

void SyntaxNode::SetSymbolAddress(Sym* addr) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeSymbol);
	value_.symbol = addr;
}

void SyntaxNode::SetLiteralAddress(Literal* addr) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeLiteral);
	value_.literal = addr;
}

const std::string& SyntaxNode::ToString() const {
	return text_;
}

SyntaxTree::SyntaxTree() 
	: root_(nullptr) {

}

SyntaxTree::~SyntaxTree() {
	PreorderTreeWalk(&SyntaxTree::DeleteTreeNode);
}

void SyntaxTree::SetRoot(SyntaxNode* root) {
	root_ = root;
}

std::string SyntaxTree::ToString() const {
	std::ostringstream oss;
	if (root_ != nullptr) {
		ToStringRecursively(oss, "", root_, true);
	}
	return oss.str();
}

void SyntaxTree::ToStringRecursively(std::ostringstream& oss, const std::string& prefix, const SyntaxNode* current, bool tail) const {
	oss << prefix << (tail ? "└─── " : "├─── ") << (current != nullptr ? current->ToString() : "null") << "\n";
	if (current == nullptr || current->GetNodeType() != SyntaxNodeOperation) {
		return;
	}

	for (int i = 0; i < current->GetChildCount(); ++i) {
		bool lastChild = current != nullptr && current->GetChildCount() == (i + 1);
		ToStringRecursively(oss, prefix + (tail ? "     " : "│    "), current->GetChild(i), lastChild);
	}

	/*
	bool tail = false;

	oss << prefix << (tail ? "└─── " : "├─── ") << (current != nullptr ? current->ToString() : "null") << "\n";
	if (current == nullptr || current->GetNodeType() != SyntaxNodeOperation) {
		return;
	}

	for (int i = 0; i < current->GetChildCount(); ++i) {
		SyntaxNodeToString(oss, prefix + (tail ? "     " : "│    "), current, current->GetChild(i));
	}
	*/
}

void SyntaxTree::DeleteTreeNode(SyntaxNode* node) {
	delete node;
}

void SyntaxTree::PreorderTreeWalk(TreeWalkCallback callback) {
	if (root_ == nullptr) {
		return;
	}

	std::stack<SyntaxNode*> s;
	s.push(root_);

	for (; !s.empty();) {
		SyntaxNode* cur = s.top();
		s.pop();

		if (cur != nullptr && cur->GetNodeType() == SyntaxNodeOperation) {
			for (int i = cur->GetChildCount() - 1; i >= 0; --i) {
				s.push(cur->GetChild(i));
			}
		}

		if (cur != nullptr) {
			(this->*callback)(cur);
		}
	}
}
