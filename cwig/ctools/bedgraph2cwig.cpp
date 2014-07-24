#include "cwig/cwig.h"
#include "utils/str_utils.h"
#include "mem/file_archive2.h"
#include "mem/info_archive.h"
#include "utils/utils.h"
#include "utils/param.h"
#include "intarray/sdarray_sml.h"

#include <boost/program_options.hpp> 

#include <cstring>
#include <tuple>
#include <fstream>
#include <iostream>

using namespace std;
using namespace app_ds;



void testbig() {
	//string inp = "D:/temp/textBigwig.bed";
	string inp = "/home/hoang/sam/data1/11N-170bp.bedGraph";
	//string inp = "/tmp/wg2V3.bedGraph";
	ifstream fi(inp.c_str());
	GenomeNumDataBuilder bd;
	bd.init(true);
	string lastchr = "";
	while (fi) {
		string line;
		getline(fi,line);
		if (line.empty()) break;
		BED_Entry e;
		e.parse(line);
		if (e.chrom != lastchr) {
			//if (lastchr.length() > 0) break;
			cout << e.chrom << endl;
			lastchr = e.chrom;
			bd.changechr(lastchr);
		}
		bd.add(e.st, e.ed, e.val);
	}
	mscds::OClassInfoArchive fo;
	bd.build(fo);
	fo.close();
	//ofstream fox("/home/hoang/sam/data1/11N-170bp.info.xml");
	ofstream fox("/tmp/info.xml");
	fox << fo.printxml() << endl;
	fox.close();

	/*GenomeNumData gd;
	bd.build(&gd);
	ofstream fox("D:/temp/dump_chr1_rlen.txt");
	gd.__testing_dump_1st_st(fox);
	fox.close();*/

	fi.close();
}

void build_with_xml(const string& input, const string& output, const string& xmlout) {
	ifstream fi(input.c_str());
	GenomeNumDataBuilder bd;
	bd.init(true);
	string lastchr = "";
	while (fi) {
		string line;
		getline(fi, line);
		if (line.empty()) continue;
		BED_Entry e;
		e.parse(line);
		if (e.chrom != lastchr) {
			//if (lastchr.length() > 0) break;
			cout << e.chrom << endl;
			lastchr = e.chrom;
			bd.changechr(lastchr);
		}
		bd.add(e.st, e.ed, e.val);
	}
	mscds::OClassInfoArchive fo;
	bd.build(fo);
	fo.close();
	//ofstream fox("/home/hoang/sam/data1/11N-170bp.info.xml");
	ofstream fox(xmlout);
	fox << fo.printxml() << endl;
	fox.close();


	fi.close();

}

void build(const string& input, const string& output, bool xml, const string& xmlout) {
	GenomeNumDataBuilder bd;
	try {
		if (!xml)
			bd.build_bedgraph(input, output);
		else {
			string out;
			if (xmlout.empty()) {
				out = output + ".xml";
			} else out = xmlout;
			build_with_xml(input, output, out);
		}
	}catch(std::exception& e) {
		std::cerr << e.what() << endl;
	}
}

int main(int argc, const char* argv[]) {
	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
		("help,h", "Print help messages")
		("input,i", po::value<std::string>()->required(), "Input bedGraph file")
		("output,o", po::value<std::string>()->required(), "Output cwig file")
		("info", po::value<std::string>()->implicit_value(""), "Produce structure XML file")
		;
	po::positional_options_description positionalOptions;
	positionalOptions.add("input", 1);
	positionalOptions.add("output", 1);
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc)
			.positional(positionalOptions).allow_unregistered().run(),
			vm);
		if (vm.count("help")) {
			cout << "Usage:\n  bedgraph2cwig <input> <output>\n" << endl;
			cout << desc << "\n";
			return 0;
		}
		po::notify(vm);
	} catch (boost::program_options::required_option& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		cout << "Usage:\n  bedgraph2cwig <input> <output>\n" << endl;
		cout << desc << "\n";
		return 1;
	} catch (boost::program_options::error& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		return 1;
	}

	Config * c = Config::getInst();
	c->parse(argc, argv);
	if (c->size() > 0) {
		cout << "params: " << endl;
		c->dump();
	}
	string xml_output;
	bool xml = false;
	if (vm.count("info")) {
		xml = true;
		xml_output = vm["info"].as<string>();
	}
	
	build(vm["input"].as<string>(), vm["output"].as<string>(), xml, xml_output);
	return 0;
}
