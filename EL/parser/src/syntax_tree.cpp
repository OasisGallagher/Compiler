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

SyntaxTree::SyntaxTree() {

}

SyntaxTree::~SyntaxTree() {

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
/*
class TreeNode {

	String name;
	List<TreeNode> children;

	public TreeNode(String name, List<TreeNode> children) {
		this.name = name;
		this.children = children;
	}

	public void print() {
		print("", true);
	}

	private void print(String prefix, boolean isTail) {
		System.out.println(prefix + (isTail ? "©¸©¤©¤ " : "©À©¤©¤ ") + name);
		for (int i = 0; i < children.size() - 1; i++) {
			children.get(i).print(prefix + (isTail ? "    " : "©¦   "), false);
		}
		if (children.size() > 0) {
			children.get(children.size() - 1).print(prefix + (isTail ? "    " : "©¦   "), true);
		}
	}
}*/
/*
ret = "\t"*level+repr(self.value)+"\n"
for child in self.children:
ret += child.__str__(level+1)
return ret
*/
std::string SyntaxTree::ToString() const {
	std::ostringstream oss;
	if (root_ != nullptr) {
		SyntaxNodeToString(oss, "", root_, false);
	}

	return oss.str();
}

void SyntaxTree::SyntaxNodeToString(std::ostringstream& oss, const std::string& prefix, SyntaxNode* current, bool tail) const {
	oss << prefix << (tail ? "©¸©¤©¤©¤ " : "©À©¤©¤©¤ ") << current->ToString() << "\n";
	for (int i = 0; i < current->ChildCount(); ++i) {
		SyntaxNodeToString(oss, prefix + (tail ? "     " : "©¦    "), current->GetChild(i), i == current->ChildCount() - 1);
	}
}