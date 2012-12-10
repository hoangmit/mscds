
#include "ReadTree.h"
#include <sstream>
#include <stack>
#include <exception>

namespace mscds {

class ReadTreeImpl {
public:

protected:

	typedef GTreeNode<LabelNode>* VexD;

	struct State {
		bool readNum;
		VexD c_child, parent;
	};
	std::stack<State> st;
	State cur;
	std::stringstream buf;

	void finishGet() {
		if (cur.readNum) {
			double x; buf >> x;
			cur.c_child->data.hasLength = true;
			cur.c_child->data.length = x;
			cur.readNum = false;
		}else {
			std::string s; buf >> s;
			cur.c_child->data.hasName = true;
			cur.c_child->data.name = s;
		}
		clearBuf();
	}

	void clearBuf() {
		buf.clear();
		buf.str("");
	}

	void nextNode() {
		cur.readNum = false;
		clearBuf();
		VexD v = new GTreeNode<LabelNode>();
		cur.parent->children.push_back(v);
		cur.c_child = v;
	}
	void newState() {
		st.push(cur);
		cur.parent = st.top().c_child;
		cur.readNum = false;
	}

public:
	ReadTreeImpl() {
	}

	VexD root;

	VexD readTree(std::istream& input) {
		root = new GTreeNode<LabelNode>();
		cur.parent = root;
		cur.c_child = root;
		cur.readNum = false;
		while (!st.empty()) st.pop(); // clear stack
		int n_open = 0, n_close = 0;
		bool stop = false;
		while (!stop && input.good() && !input.eof()) {
			char c;
			input.get(c);
			switch (c) {
				case '(':
					n_open++;
					if (cur.readNum) throw std::exception();//std::exception("Excepted \",\" or \")\"");
					newState();
					nextNode();
					break;
				case ')':
					n_close++;
					if (n_close > n_open) throw std::exception();//std::exception("Too many close brackets");
					finishGet();
					cur = st.top();
					st.pop();
					break;
				case ',' :
					finishGet();
					nextNode();
					break;
				case ';' :
					finishGet();
					stop = true;
					if (n_open - n_close != 0) throw  std::exception(); //std::exception("unbalanced brackets");
					break;
				case ':':
					if (cur.readNum) throw std::exception(); //std::exception("Excepted \",\" or \")\"");
					finishGet();
					cur.readNum = true;
					break;
				default:
					if (c != ' ' && c != '\n' && c != '\t')
						buf << c;
					break;
			}
		}
		if (input.bad())
			throw std::exception(); //std::exception("Bad input");
		if (!stop)
			throw std::exception(); //std::exception("Missing \";\" or bad input");
		return root;
	}

	/*
	VexD readTree(const std::string& s) {
		std::stringstream ss(s);
		return readTree(ss);
	}
	*/

};


GTreeNode<LabelNode>* readTree(std::istream& input) {
	ReadTreeImpl rt;
	return rt.readTree(input);
}

GTreeNode<LabelNode>* readTree(const std::string& s) {
	std::stringstream ss(s);
	return readTree(ss);
}

}//namespace