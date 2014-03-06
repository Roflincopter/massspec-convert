#include "convertdialog.hpp"
#include "measurementbounddialog.hpp"

#include <QFileDialog>

#include <sstream>
#include <iostream>

#include <cassert>

ConvertDialog::ConvertDialog(QWidget *parent) :
QDialog(parent)
{
	ui.setupUi(this);
	ui.browse_input ->setIcon(ui.browse_input->style()->standardIcon(QStyle::SP_FileDialogStart));
	ui.browse_output->setIcon(ui.browse_output->style()->standardIcon(QStyle::SP_FileDialogStart));
	
	ui.remove->setIcon(ui.remove->style()->standardIcon(QStyle::SP_DialogDiscardButton));
	ui.edit->setIcon(ui.edit->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	
	ui.edit->setEnabled(!ui.bounds->selectedItems().empty());
	ui.remove->setEnabled(!ui.bounds->selectedItems().empty());
}

std::string ConvertDialog::get_input_file_name()
{
	return ui.input_file_name->text().toStdString();
}

std::string ConvertDialog::get_output_file_name()
{
	return ui.output_file_name->text().toStdString();
}

std::vector<MeasurementBound> ConvertDialog::get_bounds()
{
	std::vector<MeasurementBound> vec;
	for(int i = 0; i < ui.bounds->count(); ++i)
	{
		vec.push_back(ui.bounds->item(i)->data(Qt::UserRole).value<MeasurementBound>());
	}
	return vec;
}

void ConvertDialog::browse_input()
{
	ui.input_file_name->setText(QFileDialog::getOpenFileName(this, "Locate the input file."));
}

void ConvertDialog::browse_output()
{
	ui.output_file_name->setText(QFileDialog::getSaveFileName(this, "Locate the output file.", ui.input_file_name->text(), "xls (*.xls)"));
}

void ConvertDialog::selection_changed()
{
	ui.edit->setEnabled(!ui.bounds->selectedItems().empty());
	ui.remove->setEnabled(!ui.bounds->selectedItems().empty());
}

void ConvertDialog::add_pressed()
{
	MeasurementBoundDialog d(this);
	d.exec();
	auto res = d.get_result();
	if(res) {
		std::stringstream ss;
		ss << *res;
		
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(ss.str()));
		QVariant var;
		var.setValue(*res);
		item->setData(Qt::UserRole, var);
		
		ui.bounds->addItem(item);
	}
}

void ConvertDialog::edit_pressed()
{
	assert(ui.bounds->selectedItems().size() == 1);
	QListWidgetItem* item = ui.bounds->selectedItems()[0];
	QVariant var = item->data(Qt::UserRole);
	MeasurementBound b = var.value<MeasurementBound>();
	
	MeasurementBoundDialog d(b.name, b.lower_bound, b.upper_bound, b.inclusive_lb, b.inclusive_ub, this);
	
	d.exec();
	auto res = d.get_result();
	if(res) {
		std::stringstream ss;
		ss << *res;
		
		item->setText(QString::fromStdString(ss.str()));
		
		QVariant var;
		var.setValue(*res);
		
		item->setData(Qt::UserRole, var);
	}
}

void ConvertDialog::delete_pressed()
{
	assert(ui.bounds->selectedItems().size() == 1);
	QListWidgetItem* item = ui.bounds->selectedItems()[0];
	delete item;
}
