#include <stack>
#include "debug.h"
#include "syntax_tree.h"

SyntaxNode::SyntaxNode(const std::string& name)
	: sibling_(nullptr), index_(0) {
	for (int i = 0; i < Constants::kMaxSyntaxNodeChildren; ++i) {
		children_[i] = nullptr;
	}

	name_ = name;
}

SyntaxNode::~SyntaxNode() {

}

void SyntaxNode::AddChild(SyntaxNode* child) {
	Assert(index_ < Constants::kMaxSyntaxNodeChildren, "out of memory");
	if (index_ > 0) {
		children_[index_ - 1]->sibling_ = child;
	}

	children_[index_++] = child;
}

SyntaxNode* SyntaxNode::GetChild(int index) {
	Assert(index >= 0 && index < Constants::kMaxSyntaxNodeChildren, "index out of range");
	return children_[index];
}

int SyntaxNode::ChildCount() const {
	return index_;
}

const std::string& SyntaxNode::ToString() const {
	return name_;
}

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
		SyntaxNodeToString(oss, "", root_);
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

void SyntaxTree::SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* current) const {
	bool tail = (current->sibling_ == nullptr);
	oss << prefix << (tail ? "└─── " : "├─── ") << current->ToString() << "\n";
	for (int i = 0; i < current->ChildCount(); ++i) {
		SyntaxNodeToString(oss, prefix + (tail ? "     " : "│    "), current->GetChild(i));
	}
}
