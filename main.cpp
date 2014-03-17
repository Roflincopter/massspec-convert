
#include "gui/convertdialog.hpp"

#include "MeasurementBound.hpp"
#include "MeasurementXlsWriter.hpp"

#include <boost/program_options.hpp>

#include <QApplication>
#include <QtPlugin>

#if defined(_WIN32) || defined(_WIN64)
	Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif //defined(_WIN32) || defined(_WIN64)

#include <string>
#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
	boost::program_options::options_description desc("Main options");
	desc.add_options()
		("help,h", "produce this help message")
		("input,i", boost::program_options::value<std::string>(), "The ascii measurement files")
		("output,o", boost::program_options::value<std::string>(), "The output xls file")
		("interval,n", boost::program_options::value<double>(), "The interval in which 2 measurement groups are taken.")
		("measurements,m", boost::program_options::value<std::vector<MeasurementBound>>()->multitoken(), "The measurement partitions with name and bounds");
	
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
	boost::program_options::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	
	std::unique_ptr<MeasurementXlsWriter> xlswriter{nullptr};
	
	if (!vm.count("input") || !vm.count("output")) {
		int fake_argc = 0;
		QApplication qapp(fake_argc, nullptr);
		
		ConvertDialog dialog;
		int exit = dialog.exec();
		
		if(exit == QDialog::Accepted) {
			xlswriter = std::unique_ptr<MeasurementXlsWriter>(new MeasurementXlsWriter(
				dialog.get_input_file_name(),
				dialog.get_output_file_name(),
				dialog.get_bounds(),
				dialog.get_interval())
			);
		} else {
			return 0;
		}
	} else {
		xlswriter = std::unique_ptr<MeasurementXlsWriter>(new MeasurementXlsWriter(
			vm["input"].as<std::string>(),
			vm["output"].as<std::string>(),
			vm["measurements"].as<std::vector<MeasurementBound>>(),
			vm["interval"].as<double>())
		);
	}
	
	try {
		xlswriter->run();
	} catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
		return 3;
	}

	return 0;
}
