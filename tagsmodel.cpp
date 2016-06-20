#include "tagsmodel.h"

#include "tracksmodel.h"

#include <QDebug>

TagsModel::TagsModel(TracksModel *database, QObject *parent):
	QAbstractItemModel(parent), mDatabase(database), mRecord(NULL)
{
	connect(mDatabase, SIGNAL(recordSelected(Record*)),
		this, SLOT(onRecordChanged(Record*)));
}

QModelIndex TagsModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex TagsModel::parent(const QModelIndex &/*child*/) const
{
	return QModelIndex();
}

int TagsModel::rowCount(const QModelIndex &parent) const
{
	if (!mRecord || parent.isValid())
		return 0;
	return mRecord->tags().size();
}

int TagsModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 1;
}

QVariant TagsModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Vertical)
		return QVariant();
	return tr("Tag");
}

bool TagsModel::isValidIndex(const QModelIndex &index) const
{
	return	!index.parent().isValid() &&
		index.column() >= 0 && index.column() < columnCount() &&
		index.row() >= 0 && index.row() < rowCount();
}

QVariant TagsModel::data(const QModelIndex &index, int role) const
{
	if (!mRecord || !isValidIndex(index) || role != Qt::DisplayRole)
		return QVariant();
	return mRecord->tags()[index.row()];
}

QString TagsModel::tagForIndex(const QModelIndex &index) const
{
	if (!mRecord || !isValidIndex(index))
		return QString();
	return mRecord->tags()[index.row()];
}

void TagsModel::onRecordChanged(Record *rec)
{
	beginResetModel();
	mRecord = rec;
	endResetModel();
}
