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
	bool extractVar(std::string line);
	std::string getPara(const std::string pname);
	std::string getPara(const char* pname);

	std::ostream* getLogger() { return _logger; }

	void reset();

	void loadFromStream(std::istream& input);
	void addPara(std::string para, std::string value);
	void addPara(const char* pname, const char* value);

	bool hasPara(const std::string name);
	int getIntPara(const std::string pname);
	int getIntPara(const char* pname);
	double getDoublePara(const std::string pname);
	double getDoublePara(const char* pname);

	static Config* getInst() {
		if(!instanceFlag) {
			_instance = new Config();
			instanceFlag = true;
		}
		return _instance;
	}

	void log(std::string msg) { *_logger << msg << std::endl; }
	void dump(std::ostream& out);

	Config();
	Config(std::istream stream);
	Config(const char* filename);
	~Config(){}

private:
	typedef std::unordered_map<std::string, std::string> MapType;

	std::ostream* _logger;
	MapType variables;
	static bool instanceFlag;
	static Config* _instance;
};


#endif //__PARAMETERS_H_
