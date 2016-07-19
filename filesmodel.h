#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QAbstractItemModel>

class Record;
class TracksModel;

class FilesModel : public QAbstractItemModel
{
	Q_OBJECT

	enum Columns {
		fileColumn = 0,
		primColumn,
		minusColumn,

		columnNumber
	};

public:
	FilesModel(TracksModel* database, QObject* parent = NULL);

	// QAbstractItemModel interface
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	QString fileForIndex(const QModelIndex& index) const;

public slots:
	void setRecord(Record* rec);
	void updateRecord();

private:
	TracksModel* mDatabase;
	Record* mRecord;
};

#endif // FILESMODEL_H
