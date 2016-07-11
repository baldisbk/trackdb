#ifndef CHECKLISTMODEL_H
#define CHECKLISTMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

class CheckListModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit CheckListModel(QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

	void setItems(const QStringList& items);
	void setChecks(const QList<bool>& checks);
	void setItemsAndChecks(const QStringList& items, const QList<bool>& checks);

	void setEditable(bool edit);

signals:
	void itemChecked(const QString& item, bool checked);

private:
	QStringList mItems;
	QList<bool> mChecks;
	bool mEditable;
};

#endif // CHECKLISTMODEL_H
