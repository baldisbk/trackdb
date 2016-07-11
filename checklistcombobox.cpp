#include "checklistcombobox.h"

#include <QItemDelegate>
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QStylePainter>

#include "checklistmodel.h"

class CheckBoxListDelegate : public QItemDelegate
{
public:

	CheckBoxListDelegate(QObject *parent): QItemDelegate(parent)
	{
	}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		bool value = index.data(Qt::CheckStateRole).toBool();

		QStyleOptionButton opt;
		opt.state |= value ? QStyle::State_On : QStyle::State_Off;
		opt.state |= QStyle::State_Enabled;
		opt.text = index.data(Qt::DisplayRole).toString();
		opt.rect = option.rect;

		QApplication::style()->drawControl(QStyle::CE_CheckBox, &opt, painter);
	}

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
	{
		return new QCheckBox(parent);
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		QCheckBox *myEditor = dynamic_cast<QCheckBox*>(editor);
		myEditor->setText(index.data(Qt::DisplayRole).toString());
		myEditor->setChecked(index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
	}

	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		QCheckBox *myEditor = dynamic_cast<QCheckBox*>(editor);
		if (myEditor)
			model->setData(index, myEditor->isChecked());
	}

	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
	{
		editor->setGeometry(option.rect);
	}
};


CheckListComboBox::CheckListComboBox(const QStringList &list, const QStringList &shortList, QWidget *parent): QComboBox(parent)
{
	mModel = new CheckListModel(this);
	mModel->setEditable(true);
	mModel->setItems(mOptions = list);
	mOptionsShort = shortList;
	int index = 0;
	foreach(QString item, list) {
		mChecked.append(false);
		if (index++ >= mOptionsShort.size())
			mOptionsShort.append(item);
	}
	setModel(mModel);
	view()->setItemDelegate(new CheckBoxListDelegate(this));
	view()->setEditTriggers(QAbstractItemView::CurrentChanged);
	view()->viewport()->installEventFilter(this);
	connect(mModel, SIGNAL(itemChecked(QString,bool)), this, SLOT(onItemChecked(QString,bool)));
}

void CheckListComboBox::setList(const QStringList &list)
{
	for (int i = 0; i < mOptions.size(); ++i)
		mChecked[i] = list.contains(mOptionsShort[i]);
	mModel->setChecks(mChecked);
}

void CheckListComboBox::setSeparator(const QString &sep)
{
	mSeparator = sep;
}

QString CheckListComboBox::stringData() const
{
	return listData().join(mSeparator);
}

QStringList CheckListComboBox::listData() const
{
	QStringList res;
	for (int i = 0; i < mOptions.size(); ++i)
		if (mChecked[i])
			res.append(mOptionsShort[i]);
	return res;
}

bool CheckListComboBox::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonRelease && object == view()->viewport())
		return true;
	return QComboBox::eventFilter(object,event);
}

void CheckListComboBox::onItemChecked(const QString &item, bool checked)
{
	int index = mOptions.indexOf(item);
	if (index == -1)
		return;
	mChecked[index] = checked;
}

void CheckListComboBox::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	painter.setPen(palette().color(QPalette::Text));

	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	opt.currentText = stringData();
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);
	painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}
