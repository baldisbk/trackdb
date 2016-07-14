#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QAbstractItemModel>

namespace Ui {
class SettingsDialog;
}

class TracksModel;

class PropertySettingsModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum Columns {
		columnName = 0,
		columnType,

		numberOfColumns,
		columnHidden,
	};

	PropertySettingsModel(TracksModel* db, QWidget *parent = 0);

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	void dbUpdated();

	QString propForIndex(const QModelIndex& index) const;
	QModelIndex indexForProp(QString prop) const;

private:
	TracksModel* mDatabase;
	bool mUpdating;
	QStringList mPropList;
};

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(TracksModel* db, QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void dbUpdated();
	void tagsUpdated();

	void propSelected(const QModelIndex& index);

	void addOrdProp();
	void addBigProp();
	void addCatProp();
	void addTag();
	void delTag();

private:
	Ui::SettingsDialog *ui;
	TracksModel* mDatabase;
	PropertySettingsModel* mPropModel;

	bool mUpdating;
};

#endif // SETTINGSDIALOG_H
