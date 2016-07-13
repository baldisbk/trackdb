#ifndef PRINTSELECTDIALOG_H
#define PRINTSELECTDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QMap>

namespace Ui {
class PrintSelectDialog;
}

class PrintSelectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PrintSelectDialog(QWidget *parent = 0);
	~PrintSelectDialog();

	void addOption(QString option);
	QStringList selected() const;
	void clear();
	void setSelected(QStringList list);

private:
	Ui::PrintSelectDialog *ui;

	QMap<QString, QCheckBox*> mChecks;
};

#endif // PRINTSELECTDIALOG_H
