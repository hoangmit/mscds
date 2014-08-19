#pragma once

/** 
Cartesian tree construction algorithm.

*/

#include "tree.h"

namespace mscds {

template<typename T>
struct CartesianTreeStruct : public BPTree<T> {
};

/// Build a cartesian tree
template<typename CTStruct>
class CartesianTreeBuilder  {
public:
	typedef typename CTStruct::NodePt NodePt;
	typedef typename CTStruct::Tp Tp;
	CTStruct& tree;
	CartesianTreeBuilder(CTStruct & _st): tree(_st) {}

	template<typename InputIterator, typename Pred>
	void build_max_tree(InputIterator bg, InputIterator ed, Pred pred) {
		//CartesianTreeStruct<T> tree;
		if (bg == ed) return ;
		InputIterator it = bg;
		auto r = tree.addNode(*it);
		tree.setRoot(r);
		++it;
		NodePt cur = tree.root;
		for (; it != ed; ++it) {
			const Tp& k = *it;
			if (pred(k, tree.getData(cur))) { // insert new leaf
				auto nn = tree.addNode(k);
				tree.setRight(cur, nn);
				cur = tree.getRight(cur);
			}else {
				while (cur != tree.nil && !(pred(k, tree.getData(cur))))
					cur = tree.getParent(cur);
				if (cur == tree.nil) { // new root
					auto nn = tree.addNode(k);
					tree.setLeft(nn, tree.root);
					tree.setRoot(nn);
					cur = nn;
				}else { // cut the edge
					auto nn = tree.addNode(k);
					tree.setLeft(nn, tree.getRight(cur));
					tree.setRight(cur, nn);
					cur = nn;
				}
			}
		}
		//return tree;
	}
};

} //namespace