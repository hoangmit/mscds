#pragma once

#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>


class Config
{
public:
	bool extractVar(const std::string& line);
	void parse(int argc, const char* argv[]);
	void loadFile(const std::string& filename);
	void loadFromStream(std::istream& input);

	std::ostream* getLogger() { return _logger; }
	void reset();

	bool hasPara(const std::string& name);
	void addPara(const std::string& para, const std::string& value);
	std::string getPara(const std::string& pname);
	int getIntPara(const std::string& pname);
	double getDoublePara(const std::string& pname);

	static Config* getInst() {
		if(!instanceFlag) {
			_instance = new Config();
			instanceFlag = true;
		}
		return _instance;
	}

	void log(std::string msg) { *_logger << msg << std::endl; }
	void dump(std::ostream& out = std::cout);

	Config();
	Config(std::istream& stream);
	Config(const std::string& filename);
	~Config(){}

private:
	typedef std::unordered_map<std::string, std::string> MapType;

	std::ostream* _logger;
	MapType variables;
	static bool instanceFlag;
	static Config* _instance;
};


#endif //__PARAMETERS_H_
