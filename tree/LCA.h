#pragma once

/** 

O(1) data structure for lowest common ancestor query

This is O(n \log n) bit data structure.

*/

#include "bitarray/bitop.h"
#include "tree.h"

#include <vector>
#include <stack>
#include <cassert>

namespace mscds {

template<typename Binder>
class DFS_Num_Mapper {
public:
	typedef unsigned int NodeId;
	typedef std::vector<NodeId> ParVec;
	typedef DFS_Num_Mapper<Binder> MyTp;
	ParVec parent;

	Binder bind;
	DFS_Num_Mapper(Binder _bind): bind(_bind) {}

	NodeId next;
	unsigned int n;

	template<typename Tree, typename Node>
	struct MyLoop {
		MyTp * thispt;
		NodeId & cur;
		Tree & t;
		MyLoop(MyTp * _thispt, Tree& _t, NodeId & _cur): thispt(_thispt), t(_t), cur(_cur) {}

		void operator()(Node v) {
			thispt->next++;
			thispt->parent.push_back(cur);
			assert(thispt->parent.size() == next + 1);
			thispt->DFS_number_rec(t, v, next);
		}
	};

	template<typename Tree, typename Node>
	void DFS_number_rec(Tree& t, Node u, NodeId cur) {
		bind(u, cur);
		MyLoop<Tree, Node> loop(this, cur);
		t.each_child(u, loop);
		/*auto & parent = this->parent;
		auto & next = this->next;
		auto thispt = this;
		t.each_child(u, [&](Node v) -> void {
			next++;
			parent.push_back(cur);
			assert(parent.size() == next + 1);
			thispt->DFS_number_rec(t, v, next);
		});*/
	}

	template<typename Tree, typename Node>
	unsigned int DFS_number(Tree& t, Node u) {
		parent.clear();
		parent.push_back(0);
		parent.push_back(0);
		next = 1;
		DFS_number_rec(t, u, 1);
		return next;
	}

	template<typename Tree, typename Node>
	void map(Tree& t, Node u) {
		//n = DFS_number(t, u);
		n = DFS_number_nonrec(t, u);
		assert(n == parent.size() - 1);
	}

	template<typename BNode>
	struct Rec {
		NodeId id;
		BNode u;
		unsigned char state;
		Rec(NodeId _p, const BNode& _u): id(_p), u(_u), state(0) {}
	};

	template<typename BTree, typename BNode>
	unsigned int DFS_number_nonrec(BTree& tree, BNode root) {
		parent.clear();
		parent.push_back(0);
		parent.push_back(0);
		auto & parent = this->parent;
		//parent, u, state
		typedef Rec<BNode> RecNode;
		std::stack<RecNode> stack;
		stack.push(RecNode(1, root));
		NodeId counter = 1;
		while (!stack.empty()) {
			auto & ti = stack.top();
			auto u = ti.u;
			
			if (ti.state == 0) {
				bind(u, ti.id);
				ti.state = 1;
			}
			if (ti.state > 0) {
				auto cc = tree.children_count(u);
				if (ti.state > cc) {
					stack.pop();
				}else {
					counter++;
					assert(parent.size() == counter);
					parent.push_back(ti.id);
					auto tistate = ti.state;
					ti.state++;
					if (ti.state > cc)
						stack.pop();
					stack.push(RecNode(counter, tree.child_at(u, tistate - 1)));
				}
			}
		}
		return counter;
	}
};

/// O(n) words, LCA data structure
struct LCA_Struct {
public:
	typedef unsigned int NodeId;
public:
	std::vector<unsigned int> A;
	std::vector<NodeId> I, L;
	std::vector<NodeId> parent;
	unsigned int n;
public:
	LCA_Struct() { n = 0; }
	void clear() {
		n = 0;
		A.clear();
		I.clear();
		L.clear();
		parent.clear();
	}

	static unsigned int find_MSB(unsigned int v) { assert(v > 0); return msb_intr(v); }
	static NodeId h(NodeId u) { assert(u > 0); return lsb_intr(u); }

	// O(1)
	unsigned int find_LCA(NodeId x, NodeId y) const {
		if (I[x] == I[y]) return std::min(x, y);
		//step 1 & 2
		NodeId k = find_MSB(I[x] ^ I[y]);
		NodeId mask = ~0 << (k + 1);
		NodeId b = (I[x] & mask) | (1 << k);
		mask = ~0 << h(b);
		NodeId j = h( (A[x] & A[y]) & mask );

		//step 3
		NodeId xbar;
		NodeId l = h(A[x]);
		if (l == j)
			xbar = x;
		else {
			mask = ~(~0 << j);
			k = find_MSB(A[x] & mask);

			mask = ~0 << (k + 1);
			NodeId Iw = (I[x] & mask) | (1 << k);
			NodeId w = L[Iw];
			xbar = parent[w];
		}

		//step 4
		NodeId ybar;
		l = h(A[y]);
		if (l == j)
			ybar = y;
		else {
			mask = ~(~0 << j);
			k = find_MSB(A[y] & mask);

			mask = ~0 << (k + 1);
			NodeId Iw = (I[y] & mask) | (1 << k);
			NodeId w = L[Iw];
			ybar = parent[w];
		}
		return std::min(xbar, ybar);
	}

};

/// From Dan Gusfield's book
class LCA_Builder {
public:
	typedef LCA_Struct::NodeId NodeId;

public:
	static unsigned int find_MSB(unsigned long v) { return msb_intr(v); }
	static NodeId h(NodeId u) { return lsb_intr(u); }

	LCA_Builder() {}

	void build(LCA_Struct& st, std::vector<NodeId>& p) {
		st.parent.swap(p);

		st.n = st.parent.size() - 1;
		assert(st.n > 0);
		assert(st.n == st.parent.size() - 1);
		st.I.resize(st.n + 1, 0);
		st.L.resize(st.n + 1, 0);
		st.A.resize(st.n + 1, 0);
		compute_I_L(st, st.n);
		compute_A(st, st.n);
	}
private:
	template<typename T1, typename T2>
	void assignMax(T1 & a, T1& b, const T1& a1, const T2& b1) {
		if (a1 >= a) {
			a = a1;
			b = b1;
		}
	}

	void compute_I_L(LCA_Struct& st, int n) {
		auto & I = st.I;
		auto & L = st.L;
		auto & parent = st.parent;
		std::vector<unsigned int> hs;
		hs.resize(n + 1, 0);
		for (int i = n; i > 0; --i) {
			auto p = parent[i];
			auto & ii = I[i];
			auto & hsi = hs[i];
			assignMax(hsi, ii, h(i), i);
			assignMax(hs[p], I[p], hsi, ii);
			st.L[ii] = i;
		}
		I[0] = 0;
		L[0] = 0;
	}

	void compute_A(LCA_Struct& st, int n) {
		auto & I = st.I;
		auto & A = st.A;
		auto & parent = st.parent;
		for (int i = 1; i <= n; ++i) {
			auto Amask = A[parent[i]];
			Amask |= 1 << h(I[i]);
			A[i] = Amask;
		}
	}
};

}//namespace