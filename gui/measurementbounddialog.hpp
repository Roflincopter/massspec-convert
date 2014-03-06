#pragma once

#include "ui_measurementbounddialog.hpp"

#include "../MeasurementBound.hpp"

#include <memory>

class MeasurementBoundDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit MeasurementBoundDialog(QWidget *parent = 0);
	explicit MeasurementBoundDialog(std::string n, double lb, double ub, double ilb, double ulb, QWidget* parent = 0);
	
	std::shared_ptr<MeasurementBound> get_result();
	
protected slots:
	void accept();
	
private:
	Ui::MeasurementBoundDialog ui;
	std::shared_ptr<MeasurementBound> bound;
};
