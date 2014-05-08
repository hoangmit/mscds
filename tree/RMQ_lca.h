#pragma once

/**
Data structure for Range Minimum/Maximum Query.

RMQ using LCA data structure.

*/

#include "LCA.h"
#include "CartesianTree.h"


namespace mscds {

class RMQ_lca_builder;

class RMQ_lca_query {
public:

private:
	friend class RMQ_lca_builder;
};


class RMQ_lca_builder {
public:
	static void build(std::vector<int>& inp, RMQ_lca_query * out, bool min_tree) {
		BTree<unsigned int> sttree;

		CartesianTreeStruct<unsigned int> treep;
		CartesianTreeBuilder<CartesianTreeStruct<unsigned int>> tbd(treep);
		tbd.build_max_tree(0, 0, 0);
		treep.toBTree(sttree);

		/*auto & o2d = st.order2dfs;
		auto & d2o = st.dfs2order;
		auto mapper = [&o2d, &d2o](unsigned int i, unsigned int u) -> void {
		o2d[i] = u;
		d2o[u] = i;
		};*/
		Mapper mapper(st.order2dfs, st.dfs2order);

		st.order2dfs.resize(st.n);
		st.dfs2order.resize(st.n + 1);

		DFS_Num_Mapper<Mapper> mp(mapper);
		mp.map(sttree, sttree.root);

		LCA_Builder lca_bd;
		lca_bd.build(st.lca, mp.parent);
	}
};


}//namespace

