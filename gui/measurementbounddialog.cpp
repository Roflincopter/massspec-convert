#include "measurementbounddialog.hpp"

#include <iostream>

MeasurementBoundDialog::MeasurementBoundDialog(QWidget *parent) :
QDialog(parent)
{
	ui.setupUi(this);

	QValidator* name_val = new QRegExpValidator(QRegExp(".+"), this);
	ui.name->setValidator(name_val);
	
	QValidator* lb_val = new QDoubleValidator(this);
	ui.lower_bound->setValidator(lb_val);
	
	QValidator* ub_val = new QDoubleValidator(this);
	ui.upper_bound->setValidator(ub_val);
}

MeasurementBoundDialog::MeasurementBoundDialog(std::string n, double lb, double ub, double ilb, double ulb, QWidget *parent)
: MeasurementBoundDialog(parent)
{
	ui.name->setText(QString::fromStdString(n));
	ui.lower_bound->setText(QString::number(lb));
	ui.inclusive_lb->setChecked(ilb);
	ui.upper_bound->setText(QString::number(ub));
	ui.inclusive_ub->setChecked(ulb);
}


std::shared_ptr<MeasurementBound> MeasurementBoundDialog::get_result()
{
	return bound;
}

void MeasurementBoundDialog::accept()
{
	bound = std::make_shared<MeasurementBound>(MeasurementBound{ui.name->text().toStdString(), ui.lower_bound->text().toDouble(), ui.upper_bound->text().toDouble(), ui.inclusive_lb->isChecked(), ui.inclusive_ub->isChecked()});
	QDialog::accept();
}
