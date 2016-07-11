#ifndef COMBOCHECKBOXDELEGATE_H
#define COMBOCHECKBOXDELEGATE_H

#include <QItemDelegate>

#define BASE_DELEGATE QItemDelegate
#define DISPLAY_ROLE Qt::UserRole
#define VALUES_ROLE (Qt::UserRole+1)

class ComboCheckBoxDelegate : public BASE_DELEGATE
{
	Q_OBJECT
public:
	explicit ComboCheckBoxDelegate(QObject *parent = 0, const QString separator = QString(", "));

	virtual QWidget *createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	void setSeparator(const QString& separator);
private:
	QString mSeparator;
};

#endif // COMBOCHECKBOXDELEGATE_H
