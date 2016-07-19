#include "filtermodel.h"

#include "tracksmodel.h"

#include <QDebug>

FilterModel::FilterModel(TracksModel *db, QObject *parent):
	QSortFilterProxyModel(parent), mDatabase(db)
{
}

void FilterModel::setFilter(QString prop, QString value)
{
	mFilterProp = prop;
	mFilterValue = value;
	invalidateFilter();
}

void FilterModel::setStateFilter(FillState state, bool on)
{
	if (on) {
		if (!mEnabledStates.contains(state))
			mEnabledStates.append(state);
	} else {
		if (mEnabledStates.contains(state))
			mEnabledStates.removeAll(state);
	}
	invalidateFilter();
}

bool FilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	QModelIndex srcStateIndex = sourceModel()->index(
		source_row, TracksModel::stateColumn, source_parent);
	if (!srcStateIndex.isValid())
		return false;
	else if (!mEnabledStates.contains(srcStateIndex.data(Qt::UserRole).toInt()))
		return false;
	if (mFilterProp.isEmpty() || mFilterValue.isEmpty())
		return true;

	Record* rec = mDatabase->recordForIndex(
		sourceModel()->index(source_row, TracksModel::idColumn, source_parent));
	if (!rec)
		return true;

	Property* prop = mDatabase->propertyType(mFilterProp);
	if (!prop) {
		int column = mDatabase->columnForProperty(mFilterProp);
		if (column == -1)
			return true;
		QModelIndex src = sourceModel()->index(
			source_row, column, source_parent);
		if (!src.isValid())
			return false;
		else
			return src.data().toString().contains(mFilterValue);
	}
	QVariant value = rec->property(mFilterProp);
	if (value.type() == QVariant::String)
		return value.toString().contains(mFilterValue);
	if (value.type() == QVariant::StringList)
		return value.toStringList().contains(mFilterValue);
	return false;
}

bool FilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if (
		left.column() == TracksModel::stateColumn &&
		right.column() == TracksModel::stateColumn)

		return left.data(Qt::UserRole) < right.data(Qt::UserRole);
	else
		return QSortFilterProxyModel::lessThan(left, right);
}
