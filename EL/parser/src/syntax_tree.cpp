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

void SyntaxNode::AddChildren(SyntaxNode** buffer, int count) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);

	value_.children = new SyntaxNode*[count + 1];
	*value_.children = (SyntaxNode*)count;

	for (int i = 1; i <= count; ++i) {
		value_.children[i] = buffer[i - 1];
		printf(buffer[i - 1]->ToString().c_str());
	}
}

SyntaxNode* SyntaxNode::GetChild(int index) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);
	Assert(index >= 0 && index < GetChildCount(), "index out of range");
	return value_.children[index + 1];
}

int SyntaxNode::GetChildCount() const {
	DEBUG_NODE_TYPE(type_, SyntaxNodeOperation);
	return (int)*value_.children;
}

void SyntaxNode::SetConstantAddress(void* addr) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeConstant);
	value_.constant = addr;
}

void SyntaxNode::SetSymbolAddress(void* addr) {
	DEBUG_NODE_TYPE(type_, SyntaxNodeSymbol);
	value_.symbol = addr;
}

const std::string& SyntaxNode::ToString() const {
	return text_;
}

/*
SyntaxTree::SyntaxTree() 
	: root_(nullptr) {

}

SyntaxTree::~SyntaxTree() {
	PreorderTreeWalk(&SyntaxTree::DeleteTreeNode);
}

SyntaxNode* SyntaxTree::AddNode(SyntaxNode* parent, const std::string& name) {
	if (parent == nullptr) {
		Assert(root_ == nullptr, "root already exists");
		return root_ = new SyntaxNode(name);
	}

	SyntaxNode* node = new SyntaxNode(name);
	parent->AddChild(node);
	return node;
}

std::string SyntaxTree::ToString() const {
	std::ostringstream oss;
	if (root_ != nullptr) {
		SyntaxNodeToString(oss, "", nullptr, root_);
	}

	return oss.str();
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

		for (int i = cur->ChildCount() - 1; i >= 0; --i) {
			s.push(cur->GetChild(i));
		}

		(this->*callback)(cur);
	}
}

void SyntaxTree::SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* parent, SyntaxNode* current) const {
	bool tail = (current + 1 == parent->children_ + parent->GetChildCount());
	oss << prefix << (tail ? "└─── " : "├─── ") << current->ToString() << "\n";
	for (int i = 0; i < current->ChildCount(); ++i) {
		SyntaxNodeToString(oss, prefix + (tail ? "     " : "│    "), current->GetChild(i));
	}
}
*/