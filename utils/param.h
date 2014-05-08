#pragma once

#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_

/** 

Classes to store program configuration and parameters

*/

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>


class Config
{
public:
	bool extractVar(const std::string& line);
	void parse(int argc, const char* argv[]);
	void parse(int argc, char* argv[]);
	void loadFile(const std::string& filename);
	void loadStream(std::istream& input);

	std::ostream* getLogger() { return _logger; }
	void reset();

	bool check(const std::string& name) const;
	Config& add(const std::string& para, const std::string& value);
	std::string get(const std::string& pname) const;
	std::string get(const std::string& pname, const std::string& defaultval) const;
	int getInt(const std::string& pname) const;
	int getInt(const std::string& pname, int defaultval) const;
	double getDouble(const std::string& pname) const;
	double getDouble(const std::string& pname, double defaultval) const;

	static Config* getInst() {
		if(!instanceFlag) {
			_instance = new Config();
			instanceFlag = true;
		}
		return _instance;
	}

	void log(const std::string& msg) { *_logger << msg << std::endl; }
	void dump(std::ostream& out = std::cout);
	size_t size() const { return variables.size(); }

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
