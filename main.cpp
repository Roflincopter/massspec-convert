
#include "MeasurementBound.hpp"
#include "MeasurementXlsWriter.hpp"

#include <boost/program_options.hpp>

#include <string>

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
