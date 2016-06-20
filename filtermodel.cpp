#include "filtermodel.h"

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



bool FilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	if (mFilterColumn == -1 || mFilterValue.isEmpty())
		return true;
	else {
		QModelIndex src = sourceModel()->index(
			source_row, mFilterColumn, source_parent);
		if (!src.isValid())
			return false;
		else
			return src.data().toString().contains(mFilterValue);
	}
}
