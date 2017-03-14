function Node(element, left, right) {
    this.element = element;
    this.level = 0;
    this.left = left;
    this.right = right;
}

function BST() {
    this.root = null;
}
BST.prototype = {
    insert: function (element) {
        var n = new Node(element, null, null);
        if (this.root == null) {
            this.root = n;
            n.level = 0;
            return true;
        } else {
            var current = this.root;
            var parent = null;
            while (current != null) {
                if (element < current.element) {
                    parent = current;
                    current = current.left;
                } else if (element > current.element) {
                    parent = current;
                    current = current.right;
                } else {
                    return false;
                }
            }
            n.level = parent.level + 1;
            if (element < parent.element) {
                parent.left = n;

            } else {
                parent.right = n;
            }
            return true;
        }
    }, toString: function () {
        return this.inOrder(this.root, function (n) {
            return "\t" + n.element + "(" + n.level + ")\t";
        });
    }, inOrder: function (n, fn) {// 中序遍历
        if (!n) {
            return "";
        } else {
            return this.inOrder(n.left, fn) + fn(n) + this.inOrder(n.right, fn);
        }
    }, preOrder: function (n, fn) { // 先序遍历
        if (!n) {
            return '';
        } else {
            return fn(n) + this.preOrder(n.left, fn) + this.preOrder(n.right, fn);
        }
    }, postOrder: function (n, fn) {// 后序遍历
        if (!n) {
            return "";
        } else {
            return this.postOrder(n.left, fn) + this.postOrder(n.right, fn) + fn(n);
        }
    }
};

var a = new BST();
a.insert(3);
a.insert(1);
a.insert(5);
a.insert(2);
a.insert(4);
console.log("inorder: " + a.toString());

var fn = function (n) {
    return "\t" + n.element + "(" + n.level + ")\t";
};
var s1 = a.preOrder(a.root, fn);
var s2 = a.postOrder(a.root, fn);
console.log("pre order:" + s1);
console.log("post order:" + s2);
