#pragma once

#include "ui_convertdialog.hpp"

#include "../MeasurementBound.hpp"
Q_DECLARE_METATYPE(MeasurementBound)

class ConvertDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit ConvertDialog(QWidget *parent = 0);
	
	std::string get_input_file_name();
	std::string get_output_file_name();
	std::vector<MeasurementBound> get_bounds();
	double get_interval();
	
private:
	Ui::ConvertDialog ui;
	
private slots:
	void browse_input();
	void browse_output();
	void selection_changed();
	void add_pressed();
	void edit_pressed();
	void delete_pressed();
};
