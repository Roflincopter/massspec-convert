
#include "excelformat/BasicExcel.hpp"

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <functional>
#include <fstream>
#include <iostream>
#include <string>


template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> const& vec)
{
	std::copy(vec.cbegin(), vec.cend() - 1, std::ostream_iterator<T>(os, ", "));
	os << vec.back();
	return os;
}

struct MeasurementBound {
	std::string name;
	double lower_bound;
	double upper_bound;
	
	bool inclusive_lb;
	bool inclusive_ub;
	
	bool contains(double reltime)
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
	
	friend std::ostream& operator<<(std::ostream& os, MeasurementBound const& m) {
		os << m.name << ' ' << (m.inclusive_lb ? '[' : '(') << m.lower_bound << ", " << m.upper_bound << (m.inclusive_ub ? ']' : ')');
	}
};

void validate(boost::any& v, std::vector<std::string> const& values, MeasurementBound*, int) {
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

class MeasurementXlsWriter 
{
	std::string input_file;
	std::string output_file;
	std::vector<MeasurementBound> bounds;
	
public:
	
	struct HeaderFormatError : public std::runtime_error
	{
		HeaderFormatError(std::string s)
		: std::runtime_error(s)
		{}
	};
	
	struct Row {
		double reltime;
		std::vector<double> values;
		
		friend std::ostream& operator<<(std::ostream& os, Row const& row)
		{
			std::cout << row.reltime << ": " << row.values;
		}
	};
	
	MeasurementXlsWriter(std::string input_file, std::string output_file, std::vector<MeasurementBound> bounds)
	: input_file(input_file)
	, output_file(output_file)
	, bounds(bounds)
	{
		if(!boost::filesystem::is_regular_file(input_file)) {
			throw std::runtime_error("File: " + input_file + " Does not exist.");
		}
	}
	
	void skip_to_input(std::istream& is)
	{
		std::string input;
		while(input != "Cycle")
		{
			is >> input;
		}
		
		//2 checks to see if the format has changed.
		is >> input;
		if(input != "Date") {
			throw HeaderFormatError("Date didn't follow Cycle.");
		}
		
		is >> input;
		if(input != "Time") {
			throw HeaderFormatError("Time didn't follow Date.");
		}
		
		std::getline(is, input);
	}
	
	Row get_row(std::istream& is)
	{
		std::string buffer;
		//Skip cycle Date Time (and AM/PM)
		for(int i = 0; i < 4; ++i)
		{
			is >> buffer;
			if (!is) {
				return {0.0d,{}};
			}
		}
		
		double reltime;
		is >> reltime;
		
		std::getline(is, buffer);
		
		double datapoint;
		std::stringstream ss(buffer);
		std::vector<double> data;
		
		while(true)
		{
			ss >> datapoint;
			if(!ss) {
				break;
			}
			data.push_back(datapoint);
		}
		return {reltime, data};
	}
	
	bool get_worksheet(double reltime, int& worksheet)
	{
		for(int i = 0; i < bounds.size(); ++i)
		{
			if(bounds[i].contains(reltime)) {
				worksheet = i;
				return true;
			}
		}
		return false;
	}
	
	void run()
	{
		std::ifstream ifs(input_file);
		skip_to_input(ifs);
		
		YExcel::BasicExcel xls;
		xls.New(bounds.size());
		
		for(int i = 0; i < bounds.size(); ++i) {
			YExcel::BasicExcelWorksheet* sheet = xls.GetWorksheet(i);
			sheet->Rename(bounds[i].name.c_str());
		}
		
		std::vector<unsigned int> rows(bounds.size(), 0);
		
		while(true) {
			Row row = get_row(ifs);
			//we've reached end of file
			if(!ifs) {
				break;
			}
			std::cout << row << std::endl;
		
			int worksheet;
			if(!get_worksheet(row.reltime, worksheet)) {
				continue;
			}
			
			YExcel::BasicExcelWorksheet* sheet = xls.GetWorksheet(worksheet);
			YExcel::BasicExcelCell* cell = sheet->Cell(rows[worksheet],0);
			cell->SetDouble(row.reltime);
			
			for(int i = 1; i <= row.values.size(); ++i)
			{
				cell = sheet->Cell(rows[worksheet],i);
				cell->SetDouble(row.values[i-1]);
			}
			
			rows[worksheet]++;
		}
		xls.SaveAs(output_file.c_str());
	}
};

int main(int argc, char* argv[])
{
	boost::program_options::options_description desc("Main options");
	desc.add_options()
		("help", "produce this help message")
		("input,i", boost::program_options::value<std::string>(), "The ascii measurement files")
		("output,o", boost::program_options::value<std::string>(), "The output xls file")
		("measurements,m", boost::program_options::value<std::vector<MeasurementBound>>()->multitoken(), "The measurement partitions with name and bounds");
	
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
	boost::program_options::notify(vm);
	
	if (!vm.count("input")) {
		std::cerr << "No input file was specified, use \"--input\"." << std::endl;
		return 1;
	}
	
	if (!vm.count("output")) {
		std::cerr << "No output file was specified, use \"--output\"." << std::endl;
		return 2;
	}
	
	MeasurementXlsWriter xlswriter(vm["input"].as<std::string>(), vm["output"].as<std::string>(), vm["measurements"].as<std::vector<MeasurementBound>>());
	try {
		xlswriter.run();
	} catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
		return 3;
	}

	return 0;
}
