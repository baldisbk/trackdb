#ifndef CHECKLISTCOMBOBOX_H
#define CHECKLISTCOMBOBOX_H

#include <QComboBox>

class CheckListModel;

class CheckListComboBox: public QComboBox
{
	Q_OBJECT
public:
	CheckListComboBox(const QStringList& list, const QStringList& shortList, QWidget* parent = 0);

	void setList(const QStringList& list);
	void setSeparator(const QString& sep);

	QString stringData() const;
	QStringList listData() const;

	bool eventFilter(QObject *object, QEvent *event);

protected:
	void paintEvent(QPaintEvent*);

private slots:
	void onItemChecked(const QString& item, bool checked);

private:
	QStringList mOptions;
	QStringList mOptionsShort;
	QList<bool> mChecked;
	QString mSeparator;
	CheckListModel* mModel;
};

#endif // CHECKLISTCOMBOBOX_H
