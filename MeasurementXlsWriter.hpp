#pragma once

#include "MeasurementBound.hpp"

#include <string>
#include <vector>

class MeasurementXlsWriter 
{
	std::string input_file;
	std::string output_file;
	std::vector<MeasurementBound> bounds;
	double measurement_group_interval;
	
public:
	
	struct HeaderFormatError : public std::runtime_error
	{
		HeaderFormatError(std::string s);
	};
	
	struct Row {
		double reltime;
		std::vector<double> values;
		
		friend std::ostream& operator<<(std::ostream& os, Row const& row);
	};
	
	MeasurementXlsWriter(std::string input_file, std::string output_file, std::vector<MeasurementBound> bounds, double measurement_group_interval);
	
	void skip_to_input(std::istream& is);
	Row get_row(std::istream& is);
	bool get_worksheet(double reltime, int& worksheet);
	void run();
};