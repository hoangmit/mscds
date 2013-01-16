#pragma once

#ifndef __COUNT2D_EXP_H_
#define __COUNT2D_EXP_H_

#include "intarray/count2d.h"

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
private:
	Count2DBuilder bd;
	Count2DQuery q;
};

struct Count2DQueryEx {
	/**
		 * @brief load the data structure into memory
		 *
		 * If `fullmemload` is false, only partial of the structure is loaded into memory
		 */
	void load(const std::string& datafile, bool fullmemload=false);


	/**
		 * @brief count return the count in the region [x1,x2) \times [y1,y2)
		 */
	unsigned int count(unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2);

	/**
		 * @brief count_grid computes the count queries for query points in a grid
		 * @param X contains the x-coordinates of the grid
		 * @param Y contains the y-coordinates of the grids
		 * @return a vector V where V[j * |X| + i] = count(i,j)
		 * e.g. V = { (x_0,y_0), (x1,y_0), (x_2,y_0) ... (x_|X|, y_0), (x_0, y_1) ... }
		 */
	std::vector<unsigned int> count_grid(const std::vector<unsigned int>& X, const
	std::vector<unsigned int>& Y);
	
	std::vector<unsigned int> Count2DQuery::heatmap(unsigned int x1, unsigned int y1, 
	unsigned int x2, unsigned int y2, unsigned int nx, unsigned int ny);

	void close();
private:
	Count2DQuery q;
};
}

#endif
