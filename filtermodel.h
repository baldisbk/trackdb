#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QSortFilterProxyModel>

#include "database.h"

class TracksModel;

class FilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	FilterModel(TracksModel* db, QObject* parent = NULL);

	void setFilter(QString prop, QString value);
	void setStateFilter(FillState state, bool on);

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
	virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
	TracksModel* mDatabase;
	QString mFilterProp;
	QString mFilterValue;
	QList<int> mEnabledStates;
};

#endif // FILTERMODEL_H
