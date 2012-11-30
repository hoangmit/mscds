%module count2d_exp
%include "std_string.i"
%include "std_vector.i"
%{
#include "count2d_exp.h"
%}

namespace count2d {
	struct Count2DBuilderEx {
		void build(const std::string& textinput, const std::string& datafile);
		void extract(const std::string& datafile, const std::string& textfile);
	};

	struct Count2DQueryEx {
		void load(const std::string& datafile);
		unsigned int count(unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2);
		std::vector<unsigned int> count_grid(const std::vector<unsigned int>& X, const std::vector<unsigned int>& Y);
		void close();
	};
}