

#include "count2d.h"
#include <cassert>
#include <vector>
#include <iostream>

using namespace mscds;
using namespace std;

#define ASSERT_EQ(x,y) assert((x)==(y))

void test1() {
	int px[10] = {0, 0, 1, 1, 4, 4, 3, 3, 5, 6};
	int py[10] = {0, 1, 0, 1, 4, 3, 4, 3, 3, 4};
	std::vector<Point> list;
	for (int i = 0; i < 10; i++) 
		list.push_back(Point(px[i], py[i]));
	Count2DBuilder bd;
	Count2DQuery cq;
	bd.build(list, &cq);
	//cout << cq.SX.to_str(true) << endl;
	//cout << cq.DPX.to_str() << endl;
	//cout << cq.SY.to_str(true) << endl;
	//cout << cq.wq.to_str() << endl;
	ASSERT_EQ(0, cq.count(0, 0));
	ASSERT_EQ(1, cq.count(1, 1));
	ASSERT_EQ(4, cq.count(2, 2));
	ASSERT_EQ(5, cq.count(4, 4));
	ASSERT_EQ(6, cq.count(5, 4));
	ASSERT_EQ(6, cq.count(4, 5));
	ASSERT_EQ(7, cq.count(6, 4));
	ASSERT_EQ(7, cq.count(7, 4));
	ASSERT_EQ(10, cq.count(7, 5));
	cout << '.' << flush;
}

void test2(unsigned int n, double p) {
	//const unsigned int n = 150;
	vector<vector<bool> > matrix;
	vector<vector<int> > count;
	for (int i = 0; i < n + 1; ++i) {
		matrix.push_back(vector<bool>());
		matrix[i].resize(n + 1, false);
		count.push_back(vector<int>());
		count[i].resize(n + 1, 0);
	}
	int rp = p * n * n;
	for (int i = 0; i < rp; i++)  {
		int x = rand() % n;
		int y = rand() % n;
		matrix[x][y] = true;
	}
	for (int i = 1; i < n+1; ++i)
		for (int j = 1; j < n+1; ++j) {
			count[i][j] = count[i-1][j] + count[i][j-1] - count[i-1][j-1];
			if (matrix[i-1][j-1]) count[i][j] += 1;
		}
	std::vector<Point> list;
	Count2DBuilder bd;

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			if (matrix[i][j])
				list.push_back(Point(i, j));
	Count2DQuery cq;
	bd.build(list, &cq);

	for (int i = 0; i < n+1; ++i)
		for (int j = 0; j < n+1; ++j) {
			int exp = count[i][j];
			int val = cq.count(i,j);
			ASSERT_EQ(exp, val);
		}
	cout << '.' << flush;
}

void test_grid_query1(unsigned int n, double p) {
	//const unsigned int n = 150;
	vector<vector<bool> > matrix;
	vector<vector<int> > count;
	for (int i = 0; i < n + 1; ++i) {
		matrix.push_back(vector<bool>());
		matrix[i].resize(n + 1, false);
		count.push_back(vector<int>());
		count[i].resize(n + 1, 0);
	}
	int rp = p * n * n;
	for (int i = 0; i < rp; i++)  {
		int x = rand() % n;
		int y = rand() % n;
		matrix[x][y] = true;
	}
	for (int i = 1; i < n+1; ++i)
		for (int j = 1; j < n+1; ++j) {
			count[i][j] = count[i-1][j] + count[i][j-1] - count[i-1][j-1];
			if (matrix[i-1][j-1]) count[i][j] += 1;
		}
	std::vector<Point> list;
	Count2DBuilder bd;

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			if (matrix[i][j])
				list.push_back(Point(i, j));
	Count2DQuery cq;
	bd.build(list, &cq);

	for (int i = 0; i < n+1; ++i)
		for (int j = 0; j < n+1; ++j) {
			int exp = count[i][j];
			int val = cq.count(i,j);
			ASSERT_EQ(exp, val);
		}
	std::vector<unsigned int> qX, qY;
	for (int i = 0; i < 4; i++)
		qX.push_back(rand() % (n+1));
	for (int i = 0; i < 5; i++)
		qY.push_back(rand() % (n+1));
	sort(qX.begin(), qX.end());
	qX.erase(unique(qX.begin(), qX.end()), qX.end());
	sort(qY.begin(), qY.end());
	qY.erase(unique(qY.begin(), qY.end()), qY.end());
	vector<vector<unsigned int> > result = cq.count_grid(qX, qY);
	for (int j = 0; j < qY.size(); j++) {
		for (int i = 0; i < qX.size(); i++) {
			if (count[qX[i]][qY[j]] != result[j][i]) {
				cout << i << " " << j << "   " << count[qX[i]][qY[j]] << " " << result[j][i] << endl;
				ASSERT_EQ(count[qX[i]][qY[j]], result[j][i]);
			}
		}
	}
	cout << '.' << flush;
}

void test_all() {
	test1();
	test2(150, 0.125);
	for (int i = 0; i < 100; ++i)
		test2(100, (1.0+(rand() % 50))/100.0);
	for (int i = 0; i < 100; ++i)
		test_grid_query1(100, (1.0+(rand() % 50))/100.0);
	cout << endl;
}

int main() {
	test_all();
	return 0;
}