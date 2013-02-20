
#include "param.h"


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <stdexcept>

using namespace std;


Config* Config::_instance = NULL;
bool Config::instanceFlag = false;

Config::Config() {
	//_logger = new ofstream("mscds.log");
	_instance = this;
	instanceFlag = true;
}

void Config::reset() {
	variables.clear();
}

Config::Config(std::istream& stream) {
	loadFromStream(stream);
	//_logger = new ofstream("ras_set.log");
	_instance = this;
	instanceFlag = true;
}

Config::Config(const std::string& filename) {
	loadFile(filename);
}

bool Config::hasPara(const std::string& pname) {
	MapType::iterator itr;
	itr = variables.find(pname);
	return  (itr != variables.end());
}

std::string Config::getPara(const std::string& pname) {
	MapType::iterator itr;
	itr = variables.find(pname);
	if (itr == variables.end())
		throw  std::runtime_error(("Cannot find variable: "+pname).c_str());
	return (*itr).second;
}

void Config::addPara(const std::string& para, const std::string& value) {
	variables[para] = value;
}


int Config::getIntPara(const std::string& pname) {
	return std::atoi(getPara(pname).c_str());
}

double Config::getDoublePara(const std::string& pname) {
	return std::atof(getPara(pname).c_str());
}

void Config::dump(std::ostream& out /* = std::cout */) {
	MapType::iterator it;
	for (it = variables.begin(); it != variables.end(); it++) {
		out << it->first << " = " << it->second << endl;
	}
}

void Config::parse(int argc, const char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		const string s = argv[i];
		if (s.length() > 3 && s[0] == '-' && s[1] == 'D')
			extractVar(s.substr(2));
		if (s.length() > 2 && s[0] == '-' && s[1] == 'F')
			loadFile(s.substr(2));
	}
}

void Config::loadFile(const std::string& filename) {
	fstream input(filename.c_str(), ios_base::in);
	if (!input.is_open()) {
		throw std::runtime_error((string("Config: Error opening file: ") + filename).c_str());
	}
	string line;
	getline(input, line);
	if (extractVar(line) && hasPara("INCLUDE_FILE")) {
		string exf = variables["INCLUDE_FILE"];
		ifstream input2(exf.c_str());
		if (!input2.is_open())
			throw std::runtime_error(("Cannot open external include file: " + exf).c_str());
		loadFromStream(input2);
		input2.close();
	}
	loadFromStream(input);
	//_logger = new ofstream("mscds.log");
	input.close();
	_instance = this;
	instanceFlag = true;
}

bool Config::extractVar(const std::string& line) {
	if (line.empty())
		return false;
	{
		int pos = (int) line.find_first_of("=");
		if (pos != (int) string::npos) {
			int p2 = (int) line.find_last_not_of(" =", pos);
			string start = line.substr(0, p2+1);
			int p3 = (int) line.find_first_not_of(" =", pos);
			string end;
			if (p3 == (int)  string::npos) end = "";
			else  end = line.substr(p3);
			//variables.insert(make_pair(start, end));
			variables[start] = end;
			return true;
		}
	}
	return false;
}

void Config::loadFromStream(std::istream& input) {
	string line;
	while (input.good()) {
		getline(input, line);
		if (!line.empty())
			extractVar(line);
	}
}
