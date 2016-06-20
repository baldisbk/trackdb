#include "filesmodel.h"

#include <QFileInfo>

#include "tracksmodel.h"

FilesModel::FilesModel(TracksModel *database, QObject *parent):
	QAbstractItemModel(parent), mDatabase(database), mRecord(NULL)
{
	connect(mDatabase, SIGNAL(recordSelected(Record*)),
		this, SLOT(setRecord(Record*)));
}

QModelIndex FilesModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex FilesModel::parent(const QModelIndex &/*child*/) const
{
	return QModelIndex();
}

int FilesModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid() || !mRecord)
		return 0;
	return mRecord->files().size();
}

int FilesModel::columnCount(const QModelIndex &/*parent*/) const
{
	return columnNumber;
}

QVariant FilesModel::data(const QModelIndex &ind, int role) const
{
	if (!mRecord || ind.row() < 0 || ind.row() >= mRecord->files().size())
		return false;
	switch (role) {
	case Qt::CheckStateRole:
		switch(ind.column()) {
		case primColumn:
			if (mRecord->main() == mRecord->files()[ind.row()])
				return Qt::Checked;
			else
				return Qt::Unchecked;
		case minusColumn:
			if (mRecord->minus() == mRecord->files()[ind.row()])
				return Qt::Checked;
			else
				return Qt::Unchecked;
		default:;
		}
		break;
	case Qt::DisplayRole:
		switch(ind.column()) {
		case fileColumn:
			return QFileInfo(mRecord->files()[ind.row()]).fileName();
		default:;
		}
		break;
	default:;
	}
	return QVariant();
}

bool FilesModel::setData(const QModelIndex &ind, const QVariant &value, int role)
{
	if (!mRecord || ind.row() < 0 || ind.row() >= mRecord->files().size())
		return false;
	if (role != Qt::CheckStateRole)
		return false;
	if (value.toInt() != Qt::Checked)
		return false;
	switch(ind.column()) {
	case primColumn:
		mDatabase->setMainFile(mRecord->files()[ind.row()]);
		emit dataChanged(index(0, primColumn), index(rowCount(), primColumn));
		return true;
	case minusColumn:
		mDatabase->setMinusFile(mRecord->files()[ind.row()]);
		emit dataChanged(index(0, minusColumn), index(rowCount(), minusColumn));
		return true;
	}
	return false;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const
{
	if (index.column() == fileColumn)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void FilesModel::setRecord(Record *rec)
{
	beginResetModel();
	mRecord = rec;
	endResetModel();
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical || role != Qt::DisplayRole)
		return QVariant();
	switch(section) {
	case fileColumn: return tr("File");
	case primColumn: return tr("Primary");
	case minusColumn: return tr("Minus");
	default:;
	}
	return QVariant();
}
