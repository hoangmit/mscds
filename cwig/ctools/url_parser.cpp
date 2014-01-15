#include "url_parser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>

BOOST_FUSION_ADAPT_STRUCT(app_ds::chrom_intv_op,
	(std::string, opname)
	(std::string, chrom)
	(unsigned int, start)
	(unsigned int, end));

namespace app_ds {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;


template <typename Iterator>
struct chrom_intv_parser : qi::grammar<Iterator, chrom_intv_op(), ascii::space_type>
{
	chrom_intv_parser() : chrom_intv_parser::base_type(start) {
		using qi::lit;
		using qi::lexeme;
		using ascii::char_;
		using qi::uint_;

		strval %= lexeme[+(char_("0-9a-zA-Z") | char_('-') | char_("._~:[]@!$()*+,;%\"\'") ) ];

		start %=
			lit("op") >> lit('=') >> strval >> lit('&') >>
			lit("chrom") >> lit('=') >> strval >> lit('&') >>
			lit("start") >> lit('=') >> uint_ >> lit('&') >>
			lit("end") >> lit('=') >> uint_;
	}

	qi::rule<Iterator, std::string(), ascii::space_type> strval;
	qi::rule<Iterator, chrom_intv_op(), ascii::space_type> start;
};

void parse(const std::string& str) {
	chrom_intv_op intv;
	typedef decltype(str.cbegin()) Itr;
	chrom_intv_parser<Itr> g;
	using ascii::space;
	bool r = qi::phrase_parse(str.cbegin(), str.cend(), g, space, intv);
	std::cout << (r ? "true" : "false") << std::endl;
	std::cout << intv.chrom << " " << intv.start << " " << intv.end << " " << intv.opname << std::endl;
}

void parse_url_query(const std::string& url, chrom_intv_op& out) {
	typedef decltype(url.cbegin()) Itr;
	chrom_intv_parser<Itr> g;
	using ascii::space;
	bool r = qi::phrase_parse(url.cbegin(), url.cend(), g, space, out);
}

void test() {
	parse("op=mean&chrom=chr1&start=10&end=100");
	//parse("chrom=chr1");

}

}
