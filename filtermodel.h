#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QSortFilterProxyModel>

#include "database.h"

class FilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	FilterModel(QObject* parent = NULL);

	void setFilter(int column, QString value);
	void setStateFilter(FillState state, bool on);

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
	virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
	int mFilterColumn;
	QString mFilterValue;
	QList<int> mEnabledStates;
};

#endif // FILTERMODEL_H
