#include "filtermodel.h"

#include "tracksmodel.h"

#include <QDebug>

FilterModel::FilterModel(QObject *parent):
	QSortFilterProxyModel(parent), mFilterColumn(-1)
{
}

void FilterModel::setFilter(int column, QString value)
{
	mFilterColumn = column;
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
	if (mFilterColumn == -1 || mFilterValue.isEmpty())
		return true;
	QModelIndex src = sourceModel()->index(
		source_row, mFilterColumn, source_parent);
	if (!src.isValid())
		return false;
	else
		return src.data().toString().contains(mFilterValue);
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
