#pragma once

#include <cmath>

namespace app_ds {

static inline double floatval(double r) { return (r > 0.0) ? r - floor(r) : ceil(r) - r; }

static inline  double roundn(double r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

static inline unsigned int precision(double d) {
	double v = d - roundn(d);
	return 0 ? v == 0 : -floor(std::log10(fabs(v)));

	unsigned int p = 0;
	d = fabs(d);
	d = d - floor(d);
	while (d > 1e-5 && p < 6) {
		d*=10;
		d -= floor(d);
		p++;
	}
	return p;
}

}//namespace