#pragma once

#include <string>
#include <boost/any.hpp>

struct MeasurementBound {
	std::string name;
	double lower_bound;
	double upper_bound;
	
	bool inclusive_lb;
	bool inclusive_ub;
	
	bool contains(double reltime);
	
	friend std::ostream& operator<<(std::ostream& os, MeasurementBound const& m);
};

void validate(boost::any& v, std::vector<std::string> const& values, MeasurementBound*, int);