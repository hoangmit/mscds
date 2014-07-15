
namespace utils {

// more precise algorithms for computing summation and variance from wiki

// Kahan summation algorithm (a.k.a.) Compensated Summation
template<typename F=double>
class CSummation {
public:
	F sum;
	CSummation() { reset(); }
	void reset() {sum = 0; c = 0;}
	void add(const F& v) {
		F y = v - c;
		F t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
private:
	F c;
};

template<typename F=double>
class IncVariance {
public:
	IncVariance() { reset(); }
	void reset() { n = 0; mean = 0; M2 = 0; }
	void add(const F& v) {
		n += 1;
		F delta = v - mean;
		mean = mean + delta/n;
		M2 = M2 + delta*(v - mean);
	}
	F variance() {
		return M2/n; //(n-1)
	}
private:
	size_t n;
	F mean, M2; 
};

}//namespace
