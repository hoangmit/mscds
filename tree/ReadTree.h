#pragma once


#include "tree.h"

#include <string>
#include <iostream>

namespace mscds {

struct LabelNode
{
	std::string name;
	bool hasName;
	bool hasLength;
	double length;

	LabelNode()
	{
		hasName = true;
		hasLength = false;
	}

	bool operator< (const LabelNode& a) const
	{
		return this->name < a.name;
	}
	
	friend std::ostream& operator<< (std::ostream& buf, const LabelNode& u) {
		if (u.hasName) buf << u.name;
		return buf;
	}
};

GTreeNode<LabelNode>* readTree(std::istream& input);
GTreeNode<LabelNode>* readTree(const std::string& s);

}//namespace