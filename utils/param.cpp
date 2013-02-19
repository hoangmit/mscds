
#include "param.h"


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>

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

bool Config::extractVar(std::string line) {
	if (line.empty())
		return false;
	if (line[0] ==  '$') {
		int pos = (int) line.find_first_of("=", 0);
		if (pos != (int) string::npos) // found
		{
			int p2 = (int) line.find_last_not_of(" =", pos);
			string start = line.substr(1, p2);
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
	while (input.good())
	{
		getline(input, line);
		if (!line.empty())
			extractVar(line);
	}
}

Config::Config(std::istream stream) {
	loadFromStream(stream);
	//_logger = new ofstream("ras_set.log");
	_instance = this;
	instanceFlag = true;
}

Config::Config(const char* filename) {
	fstream input(filename, ios_base::in);
	if (!input.is_open()) {
		throw std::exception((string("Config: Error opening file: ") + filename).c_str());
	}
	string line;
	getline(input, line);
	if (extractVar(line) && variables["INCLUDE_FILE"] != string())
	{
		string exf = variables["INCLUDE_FILE"];
		ifstream input2(exf.c_str());
		if (!input2.is_open())
			throw std::exception(("Cannot open external include file: " + exf).c_str());
		loadFromStream(input2);
		input2.close();
	}
	loadFromStream(input);
	//_logger = new ofstream("mscds.log");
	input.close();
	_instance = this;
	instanceFlag = true;
}

bool Config::hasPara(const std::string pname) {
	MapType::iterator itr;
	itr = variables.find(pname);
	return  (itr != variables.end());
}

std::string Config::getPara(const std::string pname) {
	MapType::iterator itr;
	itr = variables.find(pname);
	if (itr == variables.end())
		throw  std::exception(("Cannot find variable: "+pname).c_str());
	return (*itr).second;
}

void Config::addPara(std::string para, std::string value) {
	variables[para] = value;
}

void Config::addPara(const char* pname, const char* value) {
	addPara(string(pname), string(value));
}

std::string Config::getPara(const char* pname) {
	return getPara(string(pname));
}

int Config::getIntPara(const std::string pname) {
	return std::atoi(getPara(pname).c_str());
}

int Config::getIntPara(const char* pname) {
	return getIntPara(string(pname));
}

double Config::getDoublePara(const std::string pname) {
	return std::atof(getPara(pname).c_str());
}

double Config::getDoublePara(const char* pname) {
	return getDoublePara(string(pname));
}

void Config::dump(std::ostream& out) {
	MapType::iterator it;
	for (it = variables.begin(); it != variables.end(); it++) {
		out << '$' << it->first << " = " << it->second << endl;
	}
}
