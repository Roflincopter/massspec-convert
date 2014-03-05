
#include "MeasurementBound.hpp"

#include <boost/regex.hpp>
#include <boost/program_options.hpp>

bool MeasurementBound::contains(double reltime)
{
	bool b1 = false;
	bool b2 = false;
	
	if(inclusive_lb) {
		b1 = lower_bound <= reltime;
	} else {
		b1 = lower_bound < reltime;
	}
	
	if(inclusive_ub) {
		b2 = upper_bound >= reltime;
	} else {
		b2 = upper_bound > reltime;
	}
	
	return b1 && b2;
}


std::ostream& operator<<(std::ostream& os, const MeasurementBound& m) {
	os << m.name << ' ' << (m.inclusive_lb ? '[' : '(') << m.lower_bound << ", " << m.upper_bound << (m.inclusive_ub ? ']' : ')');
}


void validate(boost::any& v, const std::vector<std::string>& values, MeasurementBound*, int) {
	static boost::regex r(R"regex(\s*([^\b|\s]+)\s*([(|\[])\s*(\d+\.\d*)\s*,\s*(\d+\.\d*)\s*([)|\]]))regex");
	boost::program_options::validators::check_first_occurrence(v);
	
	std::string const& s = boost::program_options::validators::get_single_string(values);
	
	boost::smatch match;
	if(boost::regex_match(s, match, r)) {
		std::string name = match[1];
		double lower_bound = boost::lexical_cast<double>(match[3]);
		double upper_bound = boost::lexical_cast<double>(match[4]);
		
		bool inclusive_lb = match[2] == "[";
		bool inclusive_ub = match[4] == "]";
		
		v = boost::any(MeasurementBound{name, lower_bound, upper_bound, inclusive_lb, inclusive_ub});
		
	} else {
		throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
	}
}
