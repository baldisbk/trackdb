#ifndef PROPERTYMODEL_H
#define PROPERTYMODEL_H

#include <QAbstractItemModel>

class TracksModel;
class Record;

class PropertyModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum Columns {
		nameColumn = 0,
		valueColumn,

		columnNumber
	};

	enum FixedRows {
		titleRow = 0,
		artistRow,
		albumRow,
		tagsRow,

		fixedRowNumber
	};

	PropertyModel(TracksModel* database, QObject* parent = NULL);

	// QAbstractItemModel interface
	virtual QModelIndex index(
		int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(
		const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(
		const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual QVariant headerData(
		int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	bool isValidIndex(const QModelIndex &index) const;

public slots:
	void updateRowList();
	void updateRecord();
	void setRecord(Record *rec);

signals:
	void changed();

private:
	TracksModel* mDatabase;
	QStringList mPropNames;
	Record* mRecord;
};

#endif // PROPERTYMODEL_H
