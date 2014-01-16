#include "url_parser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>

BOOST_FUSION_ADAPT_STRUCT(app_ds::chrom_intv_op,
	(std::string, file)
	(std::string, opname)
	(std::string, chrom)
	(unsigned int, start)
	(unsigned int, end)
	(unsigned int, winsize)
	);

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

namespace app_ds {

template <typename Iterator>
struct chrom_intv_parser : qi::grammar<Iterator, chrom_intv_op()>
{
	chrom_intv_parser() : chrom_intv_parser::base_type(start) {
		using qi::lit;
		using qi::lexeme;
		using ascii::char_;
		using qi::uint_;
		using qi::_val;
		using phoenix::at_c;
		strval %= lexeme[+(char_("0-9a-zA-Z") | char_('-') | char_("._~:[]@!$()*+,;%\"\'") ) ];

		start %=
			qi::eps[at_c<5>(_val) = 1] >>
			qi::omit[*(char_ - char_('?'))] >> '?' >>
			lit("file")  >> '=' >> strval >> '&' >>
			lit("op")    >> '=' >> strval >> '&' >>
			lit("chrom") >> '=' >> strval >> '&' >>
			lit("start") >> '=' >> uint_ >> '&' >>
			lit("end")   >> '=' >> uint_ >> -(lit('&') >> 'w' >> '=' >> uint_);
	}

	qi::rule<Iterator, std::string()> strval;
	qi::rule<Iterator, chrom_intv_op()> start;
};

bool parse_url_query(const std::string& url, chrom_intv_op& out) {
	typedef decltype(url.cbegin()) Itr;
	chrom_intv_parser<Itr> g;
	using qi::blank;
	bool r = qi::phrase_parse(url.cbegin(), url.cend(), g, blank, out);
	return r;
}



}
