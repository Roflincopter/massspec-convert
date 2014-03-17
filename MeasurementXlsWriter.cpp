
#include "MeasurementXlsWriter.hpp"
#include "container_util.hpp"

#include "excelformat/BasicExcel.hpp"

#include <QApplication>

#include <boost/filesystem.hpp>
#include <sstream>

MeasurementXlsWriter::HeaderFormatError::HeaderFormatError(std::string s)
: std::runtime_error(s)
{}


std::ostream& operator<<(std::ostream& os, const MeasurementXlsWriter::Row& row)
{
	std::cout << row.reltime << ": " << row.values;
}


MeasurementXlsWriter::MeasurementXlsWriter(std::string input_file, std::string output_file, std::vector<MeasurementBound> bounds, double measurement_group_interval)
    : input_file(input_file)
    , output_file(output_file)
    , bounds(bounds)
    , measurement_group_interval(measurement_group_interval)
{
	if(!boost::filesystem::is_regular_file(input_file)) {
		throw std::runtime_error("File: " + input_file + " Does not exist.");
	}
}

void MeasurementXlsWriter::skip_to_input(std::istream& is)
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


MeasurementXlsWriter::Row MeasurementXlsWriter::get_row(std::istream& is)
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

bool MeasurementXlsWriter::get_worksheet(double reltime, int& worksheet)
{
	/* For when the interval is always the max upper_bound.
	auto comp = [](MeasurementBound const& lh, MeasurementBound const& rh)
		{
			return lh.upper_bound < rh.upper_bound;
		};
	
	double max_upper_bound = std::max_element(bounds.cbegin(), bounds.cend(), comp)->upper_bound;
	*/
	for(int i = 0; i < bounds.size(); ++i) {
		if(bounds[i].contains(fmod(reltime, measurement_group_interval))) {
			worksheet = i;
			return true;
		}
	}
	return false;
}


void MeasurementXlsWriter::run()
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
		
		int worksheet;
		if(!get_worksheet(row.reltime, worksheet)) {
			continue;
		}
		
		YExcel::BasicExcelWorksheet* sheet = xls.GetWorksheet(worksheet);
		YExcel::BasicExcelCell* cell = sheet->Cell(rows[worksheet],0);
		cell->SetDouble(row.reltime);
		
		for(int i = 1; i <= row.values.size(); ++i) {
			cell = sheet->Cell(rows[worksheet],i);
			cell->SetDouble(row.values[i-1]);
		}
		
		rows[worksheet]++;
	}
	xls.SaveAs(output_file.c_str());
}
