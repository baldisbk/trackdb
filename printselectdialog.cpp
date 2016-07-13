#include "printselectdialog.h"
#include "ui_printselectdialog.h"

PrintSelectDialog::PrintSelectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PrintSelectDialog)
{
	ui->setupUi(this);
}

PrintSelectDialog::~PrintSelectDialog()
{
	delete ui;
}

void PrintSelectDialog::addOption(QString option)
{
	ui->verticalLayout->insertWidget(
		ui->verticalLayout->count()-1,
				mChecks[option] = new QCheckBox(option, this));
}

QStringList PrintSelectDialog::selected() const
{
	QStringList res;
	if (ui->checkBox->isChecked())
		res.append(QString());
	foreach(QCheckBox* check, mChecks)
		if (check->isChecked())
			res.append(check->text());
	return res;
}

void PrintSelectDialog::clear()
{
	foreach(QCheckBox* check, mChecks) {
		ui->verticalLayout->removeWidget(check);
		delete check;
	}
	mChecks.clear();
}

void PrintSelectDialog::setSelected(QStringList list)
{
	foreach(QCheckBox* check, mChecks)
		check->setChecked(false);
	foreach(QString prop, list)
		if (mChecks.contains(prop))
			mChecks[prop]->setChecked(true);
	ui->checkBox->setChecked(list.contains(QString()));
}
