#include "combocheckboxdelegate.h"

#include "checklistcombobox.h"

ComboCheckBoxDelegate::ComboCheckBoxDelegate(QObject *parent, const QString separator)
	: BASE_DELEGATE(parent), mSeparator(separator)
{
}

QWidget *ComboCheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QVariant listv = index.model()->data(index, DISPLAY_ROLE);
	if (listv.isNull())
		return BASE_DELEGATE::createEditor(parent, option, index);

	QStringList list = listv.toStringList();
	QStringList shortList = index.model()->data(index, VALUES_ROLE).toStringList();

	CheckListComboBox* widget = new CheckListComboBox(list, shortList, parent);
	widget->setSeparator(mSeparator);
	return widget;
}

void ComboCheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QVariant listv = index.model()->data(index, DISPLAY_ROLE);
	if (listv.isNull())
		return BASE_DELEGATE::setEditorData(editor, index);
	QStringList list = index.model()->data(index, Qt::EditRole).toStringList();
	CheckListComboBox* widget = dynamic_cast<CheckListComboBox*>(editor);
	widget->setList(list);
}

void ComboCheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QVariant listv = index.model()->data(index, DISPLAY_ROLE);
	if (listv.isNull())
		return BASE_DELEGATE::setModelData(editor, model, index);
	CheckListComboBox* widget = dynamic_cast<CheckListComboBox*>(editor);
	model->setData(index, widget->listData());
}

void ComboCheckBoxDelegate::setSeparator(const QString& separator)
{
	mSeparator = separator;
}
