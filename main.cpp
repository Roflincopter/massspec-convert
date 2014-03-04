
#include "excelformat/BasicExcel.hpp"

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <functional>
#include <fstream>
#include <iostream>
#include <string>

struct Measurement {
	std::string name;
	double lower_bound;
	double upper_bound;
	
	bool inclusive_lb;
	bool inclusive_ub;
	
	friend std::ostream& operator<<(std::ostream& os, Measurement const& m) {
		os << m.name << ' ' << (m.inclusive_lb ? '[' : '(') << m.lower_bound << ", " << m.upper_bound << (m.inclusive_ub ? ']' : ')');
	}
};

void validate(boost::any& v, std::vector<std::string> const& values, Measurement*, int) {
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
		
		v = boost::any(Measurement{name, lower_bound, upper_bound, inclusive_lb, inclusive_ub});
		
	} else {
		throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
	}
}

class MeasurementXlsWriter 
{
	std::string input_file;
	std::string output_file;
	std::vector<Measurement> bounds;
	
public:
	MeasurementXlsWriter(std::string input_file, std::string output_file, std::vector<Measurement> bounds)
	: input_file(input_file)
	, output_file(output_file)
	, bounds(bounds)
	{
		if(!boost::filesystem::is_regular_file(input_file)) {
			throw std::runtime_error("File: " + input_file + " Does not exist.");
		}
	}
	
	void run()
	{
		YExcel::BasicExcel xls;
		xls.New(bounds.size());
		
		YExcel::BasicExcelWorksheet& sheet = *xls.GetWorksheet(0);
		sheet.Rename("Yippie");
		
		YExcel::BasicExcelCell& cell = *sheet.Cell(0,0);
		cell.SetDouble(20.10d);
		
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
		("measurements,m", boost::program_options::value<std::vector<Measurement>>()->multitoken(), "The measurement partitions with name and bounds");
	
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
	
	MeasurementXlsWriter xlswriter(vm["input"].as<std::string>(), vm["output"].as<std::string>(), vm["measurements"].as<std::vector<Measurement>>());
	try {
		xlswriter.run();
	} catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
		return 3;
	}

	return 0;
}
