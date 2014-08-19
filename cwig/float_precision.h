#pragma once

#include <cmath>
#include <ctgmath>

namespace app_ds {

/// estimate the number of floating digits of a number
static inline unsigned int fprecision(double d) {
	double intpart;
	static const double eps = 1e-5;
	unsigned int p = 0;
	d = fabs(d);
	d = modf(d, &intpart);
	while (d > eps && 1.0 - d > eps && p < 7) {
		d *= 10;
		d = modf(d, &intpart);
		p++;
	}
	return p;
}

}//namespace