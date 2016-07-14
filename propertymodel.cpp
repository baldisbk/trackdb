#include "propertymodel.h"

#include "tracksmodel.h"

PropertyModel::PropertyModel(TracksModel *database, QObject *parent):
	QAbstractItemModel(parent), mDatabase(database), mRecord(NULL)
{
	connect(mDatabase, SIGNAL(dbChanged()), this, SLOT(updateRowList()));
	connect(mDatabase, SIGNAL(recordChanged()), this, SLOT(updateRecord()));
	connect(mDatabase, SIGNAL(recordSelected(Record*)),
		this, SLOT(setRecord(Record*)));
	updateRowList();
}

QModelIndex PropertyModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex PropertyModel::parent(const QModelIndex &/*child*/) const
{
	return QModelIndex();
}

int PropertyModel::rowCount(const QModelIndex &parent) const
{
	if (!mRecord || parent.isValid())
		return 0;
	return mPropNames.size() + fixedRowNumber;
}

int PropertyModel::columnCount(const QModelIndex &/*parent*/) const
{
	return columnNumber;
}

QVariant PropertyModel::data(const QModelIndex &index, int role) const
{
	if (!isValidIndex(index) || !mRecord)
		return QVariant();
	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case nameColumn:
			switch(index.row()) {
			case titleRow: return tr("Title");
			case artistRow: return tr("Artist");
			case albumRow: return tr("Album");
			case tagsRow: return tr("Tags");
			default: return mPropNames[index.row() - fixedRowNumber];
			}
		case valueColumn:
			switch(index.row()) {
			case titleRow: return mRecord->title;
			case artistRow: return mRecord->artist;
			case albumRow: return mRecord->album;
			case tagsRow: return mRecord->tags().join(", ");
			default: {
				QVariant prop = mRecord->property(
					mPropNames[index.row() - fixedRowNumber]);
				if (prop.type() == QVariant::String)
					return prop;
				else if (prop.type() == QVariant::StringList)
					return prop.toStringList().join(", ");
				else
					return QVariant();
			}
			}
		}
	case Qt::EditRole:
		if (index.column() != valueColumn)
			return QVariant();
		switch(index.row()) {
		case titleRow: return mRecord->title;
		case artistRow: return mRecord->artist;
		case albumRow: return mRecord->album;
		case tagsRow: return mRecord->tags();
		default: return mRecord->property(
				mPropNames[index.row() - fixedRowNumber]);
		}
	case Qt::UserRole:
		if (index.column() != valueColumn)
			return QVariant();
		switch(index.row()) {
		case titleRow:
		case artistRow:
		case albumRow:
			return QVariant();
		case tagsRow:
			return mDatabase->allTags();
		default: {
			QString pName = mPropNames[index.row() - fixedRowNumber];
			if (mDatabase->propertyType(pName)->type == Property::Category)
				return mDatabase->allCategories(pName);
			else
				return QVariant();
		}
		}
	}
	return QVariant();
}

bool PropertyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!isValidIndex(index) || !mRecord)
		return false;
	if (role != Qt::EditRole || index.column() != valueColumn)
		return false;
	switch(index.row()) {
	case titleRow: mRecord->title = value.toString(); break;
	case artistRow: mRecord->artist = value.toString(); break;
	case albumRow: mRecord->album = value.toString(); break;
	case tagsRow: mRecord->setProperty(QString(), value);
	default: {
		if (!mRecord->setProperty(
			mPropNames[index.row() - fixedRowNumber],
			value))
			return false;
		break;
	}
	}
	mDatabase->updateRecord();

	emit dataChanged(index, index);
	return true;
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical || role != Qt::DisplayRole)
		return QVariant();
	switch (section) {
	case nameColumn: return tr("Name");
	case valueColumn: return tr("Value");
	default:;
	}
	return QVariant();
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &index) const
{
	if (index.column() == nameColumn)
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	else
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool PropertyModel::isValidIndex(const QModelIndex &index) const
{
	return	!index.parent().isValid() &&
		index.column() >= 0 && index.column() < columnCount() &&
			index.row() >= 0 && index.row() < rowCount();
}

void PropertyModel::updateRowList()
{
	beginResetModel();
	mPropNames = mDatabase->allProperties();
	endResetModel();
}

void PropertyModel::updateRecord()
{
	setRecord(mRecord);
}

void PropertyModel::setRecord(Record *rec)
{
	beginResetModel();
	mRecord = rec;
	endResetModel();
}
