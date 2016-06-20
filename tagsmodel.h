#ifndef TAGSMODEL_H
#define TAGSMODEL_H

#include <QAbstractItemModel>

class Record;
class TracksModel;

class TagsModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	TagsModel(TracksModel* database, QObject* parent = NULL);

	// QAbstractItemModel interface
	virtual QModelIndex index(
		int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(
		const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(
		int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	bool isValidIndex(const QModelIndex &index) const;
	QString tagForIndex(const QModelIndex& index) const;

public slots:
	void onRecordChanged(Record* rec);
private:
	TracksModel* mDatabase;
	Record* mRecord;
};

#endif // TAGSMODEL_H
