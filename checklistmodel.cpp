#include "checklistmodel.h"

CheckListModel::CheckListModel(QObject *parent): QAbstractItemModel(parent), mEditable(false)
{
}

QVariant CheckListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= rowCount())
		return QVariant();

	switch (role) {
	case Qt::DisplayRole: return mItems.at(index.row());
	case Qt::CheckStateRole: return mChecks.at(index.row())?Qt::Checked:Qt::Unchecked;
	default:;
	}

	return QVariant();
}

Qt::ItemFlags CheckListModel::flags(const QModelIndex&) const
{
	if (mEditable)
		return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
	else
		return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

int CheckListModel::columnCount(const QModelIndex &) const
{
	return 1;
}

int CheckListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return mItems.size();
}

QModelIndex CheckListModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

QModelIndex CheckListModel::index(int row, int column, const QModelIndex &parent) const
{
	if (row >= rowCount(parent) || row < 0 || column >= columnCount(parent) || column < 0 || parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

bool CheckListModel::setData(const QModelIndex& index, const QVariant& value, int)
{
	if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= rowCount())
		return false;
	if (mChecks.at(index.row()) == value.toBool())
		return false;
	mChecks[index.row()] = value.toBool();
	emit itemChecked(mItems.at(index.row()), mChecks.at(index.row()));
	emit dataChanged(index, index);
	return true;
}

void CheckListModel::setItems(const QStringList &items)
{
	beginResetModel();
	mItems = items;
	while (mItems.size() > mChecks.size())
		mChecks.append(false);
	endResetModel();
}

void CheckListModel::setChecks(const QList<bool> &checks)
{
	beginResetModel();
	mChecks = checks;
	while (mItems.size() > mChecks.size())
		mChecks.append(false);
	endResetModel();
}

void CheckListModel::setItemsAndChecks(const QStringList &items, const QList<bool> &checks)
{
	beginResetModel();
	mItems = items;
	mChecks = checks;
	while (mItems.size() > mChecks.size())
		mChecks.append(false);
	endResetModel();
}

void CheckListModel::setEditable(bool edit)
{
	mEditable = edit;
}
