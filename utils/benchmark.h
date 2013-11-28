#pragma once

#include <functional>
#include <chrono>
#include <list>
#include <vector>

template<typename SharedFixture>
class Benchmarker {
public:
	typedef void(*FuncType)(SharedFixture*);
	//typedef std::function<void(Fixture*)> FuncType;

private:
	struct FuncInfo {
		std::string name;
		FuncType func;
		unsigned int nrun;
		FuncInfo() : func(NULL), nrun(0) {}
		FuncInfo(const std::string& _name, FuncType f, unsigned int _nrun) : name(_name), func(f), nrun(_nrun) {}
	};

	std::list<FuncInfo> lst;

	typedef std::vector<std::pair<std::string, double> > RESVector;
	RESVector results;
	std::vector<RESVector> allres;
public:
	Benchmarker() : n_samples(1), verbose(true) {}

	unsigned int n_samples;
	bool verbose;
	std::vector<int> problemSizes;

	void set_sizes(const std::vector<int>& sizes) {
		problemSizes = sizes;
	}

	void add(const std::string& name, FuncType fc, unsigned int nrun = 1){
		lst.emplace_back(name, fc, nrun);
	}

	void run_all() {
		if (problemSizes.empty())
			_run_methods(-1, results);
		else {
			allres.resize(problemSizes.size());
			unsigned int i = 0;
			for (auto& v : problemSizes) {
				_run_methods(v, allres[i]);
				++i;
			}
		}
	}

	void _run_methods(int size, RESVector& results) {
		results.resize(lst.size());
		unsigned int idx = 0;
		for (auto& fc : lst) {
			results[idx].first = fc.name;
			idx++;
		}

		typedef std::chrono::high_resolution_clock Clock;
		typedef std::chrono::duration<double, std::milli> millisecs_t;

		for (unsigned int sample = 0; sample < n_samples; ++sample) {
			SharedFixture qfx;
			unsigned int idx = 0;
			qfx.SetUp(size);
			for (FuncInfo& fc : lst) {
				Clock::duration d;
				if (verbose) cout << fc.name << endl;
				unsigned int rc = fc.nrun;
				if (rc > 1) {
					auto t1 = Clock::now();
					while (rc) {
						fc.func(&qfx);
						--rc;
					}
					auto t2 = Clock::now();
					d += t2 - t1;
				}
				else {
					auto t1 = Clock::now();
					fc.func(&qfx);
					auto t2 = Clock::now();
					d += t2 - t1;
				}
				results[idx].second += chrono::duration_cast<millisecs_t>(d).count() / fc.nrun;
				idx++;
			}
			qfx.TearDown();
		}
		for (auto& r : results) {
			r.second /= (n_samples);
		}
	}

	void report(int baseline = -1) {
		if (problemSizes.empty()) {
			cout << endl;
			_report_methods(results, baseline);
		} else {
			unsigned int i = 0;
			for (auto& v : problemSizes) {
				cout << endl;
				cout << "Problem size: " << v << endl;
				_report_methods(allres[i], baseline);
				++i;
			}
		}
	}

	void _report_methods(const RESVector& results, int baseline = -1) {
		if (baseline >= 0 && baseline < results.size()) {
			double baseval = results[baseline].second;
			std::cout << "Baseline method : " << results[baseline].first << std::endl;
			std::cout << "Baseline value  = " << baseval << std::endl;
			for (auto& r : results) {
				std::cout << r.first << " \t" << r.second / baseval << std::endl;
			}
		}
		else {
			for (auto& r : results) {
				std::cout << r.first << " \t" << r.second << std::endl;
			}
		}
	}
};
