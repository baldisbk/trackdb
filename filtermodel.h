#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QSortFilterProxyModel>

class FilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	FilterModel(QObject* parent = NULL);

	void setFilter(int column, QString value);

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
	int mFilterColumn;
	QString mFilterValue;
};

#endif // FILTERMODEL_H
