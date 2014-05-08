#pragma once

/** 

O(n \log n)-bit tree using pointers.

*/

#include <vector>
#include <iostream>
#include <algorithm>

namespace mscds {


template<typename T>
struct GTreeNode {
	std::vector<GTreeNode<T>*> children;
	T data;

	static void free(GTreeNode<T>* u) {
		auto & lst = u->children;
		for (auto it = lst.begin(); it != lst.end(); ++it) {
			free(*it);
		}
		u->children.clear();
		delete(u);
	}

	void printR(std::ostream& fo, GTreeNode<T>* u) {
		if (u->children.size() == 0) {
			fo << u->data;
		}else {
			auto & r = u->children;
			fo << '(';
			bool firsttime = true;
			for (auto i = r.begin(); i != r.end(); ++i) {
				if (!firsttime) fo << ',';
				else { firsttime = false; }
				printR(fo, *i);
			}
			fo << ')';
			fo << u->data;
		}
	}

	void print(std::ostream& fo) {
		printR(fo, this);
		std::cout << ';' << std::endl;
	}

	void print() {
		print(std::cout);
	}

};


struct GTreeStub {
	template<typename T, typename Pred>
	void each_child(GTreeNode<T>* u, Pred p) const {
		for (auto it = u->children.begin(); it != u->children.end(); ++it) {
			p(*it);
		}
	}

	template<typename T>
	unsigned int children_count(GTreeNode<T>* u) const {
		return u->children.size();
	}

	template<typename T>
	GTreeNode<T>* child_at(GTreeNode<T>* u, unsigned int i) {
		return u->children[i];
	}
};

template <typename T>
struct BTree {
	typedef unsigned int NodePt;
	std::vector<T> store;
	std::vector<std::pair<NodePt, NodePt>> children;

	NodePt nil;
	NodePt root;

	BTree() {
		nil = -1;
		root = nil;
	}

	BTree(const BTree& a): store(a.store),
		children(a.children), nil(a.nil), root(a.root) {
	}

	NodePt getRoot() const {
		return root;
	}
	void setRoot(NodePt newroot) {
		root = newroot;
	}

	unsigned int children_count(NodePt u) {
		return (hasLeft(u) ? 1 : 0) + (hasRight(u) ? 1 : 0);
	}

	NodePt child_at(NodePt u, unsigned int i) {
		if (i == 0) {
			if (hasLeft(u)) return getLeft(u);
			else return getRight(u);
		}else {
			if (hasLeft(u)) return getRight(u);
			else return nil;
		}
	}

	NodePt addNode() {
		store.push_back(T());
		children.push_back(std::make_pair(nil, nil));
		return store.size() - 1;
	}

	NodePt addNode(const T& a) {
		store.push_back(a);
		children.push_back(std::make_pair(nil, nil));
		return store.size() - 1;
	}

	NodePt getLeft(NodePt u) {
		return children[u].first;
	}

	NodePt getRight(NodePt u) {
		return children[u].second;
	}
	const NodePt getLeft(NodePt u) const {
		return children[u].first;
	}

	const NodePt getRight(NodePt u) const {
		return children[u].second;
	}

	bool hasLeft(NodePt u) const {
		return children[u].first != nil;
	}

	bool hasRight(NodePt u) const {
		return children[u].second != nil;
	}

	T& getData(NodePt u) {
		return store[u];
	}

	T& leftData(NodePt u) {
		return getData(getLeft(u));
	}

	T& rightData(NodePt u) {
		return getData(getRight(u));
	}

	const T& getData(NodePt u) const {
		return store[u];
	}
	const T& leftData(NodePt u) const {
		return getData(getLeft(u));
	}

	const T& rightData(NodePt u) const {
		return getData(getRight(u));
	}

	void setLeft(NodePt u, NodePt v) {
		children[u].first = v;
	}

	template<typename Pred>
	void each_child(NodePt u, Pred p) {
		if (hasLeft(u))
			p(getLeft(u));
		if (hasRight(u))
			p(getRight(u));
	}

	void setRight(NodePt u, NodePt v) {
		children[u].second = v;
	}

	bool is_leaf(NodePt u) const {
		return !hasLeft(u) && !hasRight(u);
	}

	void clear() {
		nil = -1;
		root = nil;
		children.clear();
		store.clear();
	}

	void printR(std::ostream& fo, NodePt u) {
		if (!hasLeft(u) && !hasRight(u)) {
			fo << getData(u);
		}else {
			fo << '(';
			if (hasLeft(u)) printR(fo, getLeft(u));
			fo << ',';
			if (hasRight(u)) printR(fo, getRight(u));
			fo << ')';
			fo << getData(u);
		}
	}

	void print(std::ostream& fo) {
		printR(fo, root);
		fo << ';' << std::endl;
	}

	void print() {
		print(std::cout);
	}
};

template<typename T>
struct BPTree : public BTree<T> {
	typedef typename BTree<T>::NodePt NodePt;
	typedef T Tp;
	std::vector<NodePt> parent;
	BPTree() {}
	BPTree(const BPTree<T>& a): BTree<T>(a), parent(a.parent) {}

	NodePt addNode() {
		parent.push_back(BTree<T>::nil);
		return BTree<T>::addNode();
	}

	NodePt addNode(const T& a) {
		parent.push_back(BTree<T>::nil);
		return BTree<T>::addNode(a);
	}

	NodePt getParent(NodePt u) {
		return parent[u];
	}

	void setLeft(NodePt u, NodePt v) {
		BTree<T>::setLeft(u, v);
		parent[v] = u;
	}

	void setRight(NodePt u, NodePt v) {
		BTree<T>::setRight(u, v);
		parent[v] = u;
	}

	void clear() {
		BTree<T>::clear();
		parent.clear();
	}

	void toBTree(BTree<T>& t) {
		t.clear();
		t.children.swap(this->children);
		t.store.swap(this->store);
		t.root = this->root;
		parent.clear();
	}
};

} // namespace