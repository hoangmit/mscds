#pragma once

#ifndef __COUNT2D_EXP_H_
#define __COUNT2D_EXP_H_

#include <string>
#include <vector>

namespace mscds {
	struct Count2DBuilderEx {
		/** \brief build the Count2D data structure.
		 *
		 * The first line of the input is `N` the number of points. Each of the
		 * `N` subsequence lines contains two non-negatives integers specify
		 * the coordinates of the points
		 * e.g
		 * 3
		 * 1 1
		 * 2 2
		 * 3 3
		 *
		 */
		void build(const std::string& textinput, const std::string& datafile);

		/**
		 * \brief convert the data file back to the text file format.
		 */
		void extract(const std::string& datafile, const std::string& textfile);
	};

	struct Count2DQueryEx {
		void load(const std::string& datafile, bool fullmemload=false);
		unsigned int count(unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2);
		std::vector<unsigned int> count_grid(const std::vector<unsigned int>& X, const std::vector<unsigned int>& Y);
		void close();
	};
}

#endif
