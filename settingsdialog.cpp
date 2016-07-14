#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "tracksmodel.h"

SettingsDialog::SettingsDialog(TracksModel *db, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog), mDatabase(db), mUpdating(false)
{
	ui->setupUi(this);
	connect(db, SIGNAL(dbChanged()), this, SLOT(dbUpdated()));
	connect(db, SIGNAL(tagsChanged()), this, SLOT(tagsUpdated()));
	ui->propView->setModel(mPropModel = new PropertySettingsModel(db, this));
	connect(ui->propView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		this, SLOT(propSelected(QModelIndex)));
	connect(ui->addBigButton, SIGNAL(clicked(bool)), this, SLOT(addBigProp()));
	connect(ui->addCatButton, SIGNAL(clicked(bool)), this, SLOT(addCatProp()));
	connect(ui->addOrdButton, SIGNAL(clicked(bool)), this, SLOT(addOrdProp()));
	connect(ui->addTagButton, SIGNAL(clicked(bool)), this, SLOT(addTag()));
	connect(ui->removeTagButton, SIGNAL(clicked(bool)), this, SLOT(delTag()));

	setModal(true);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::dbUpdated()
{
	if (mUpdating) return;
	mPropModel->dbUpdated();
	tagsUpdated();
}

void SettingsDialog::tagsUpdated()
{
	propSelected(ui->propView->currentIndex());
}

void SettingsDialog::propSelected(const QModelIndex &index)
{
	ui->tagView->clear();
	QString prop = mPropModel->propForIndex(index);
	Property* p = mDatabase->propertyType(prop);
	bool hasTags = !p || p->type == Property::Category;
	ui->tagWidget->setVisible(hasTags);
	QStringList tags = mDatabase->allCategories(mPropModel->propForIndex(index));
	ui->tagView->setColumnCount(1);
	ui->tagView->setRowCount(tags.size());
	ui->tagView->setHorizontalHeaderItem(0, new QTableWidgetItem("Tag"));
	int row = 0;
	foreach(QString tag, tags)
		ui->tagView->setItem(row++, 0, new QTableWidgetItem(tag));
}

void SettingsDialog::addOrdProp()
{
	if (mDatabase->addProperty(ui->propEdit->text(), Property::Ordinary))
		ui->propView->setCurrentIndex(
			mPropModel->indexForProp(ui->propEdit->text()));
}

void SettingsDialog::addBigProp()
{
	if (mDatabase->addProperty(ui->propEdit->text(), Property::Big))
		ui->propView->setCurrentIndex(
			mPropModel->indexForProp(ui->propEdit->text()));
}

void SettingsDialog::addCatProp()
{
	if (mDatabase->addProperty(ui->propEdit->text(), Property::Category))
		ui->propView->setCurrentIndex(
			mPropModel->indexForProp(ui->propEdit->text()));
}

void SettingsDialog::addTag()
{
	if (!ui->propView->currentIndex().isValid())
		return;
	QString tag = ui->tagEdit->text();
	QString prop = mPropModel->propForIndex(ui->propView->currentIndex());
	mDatabase->addNewTag(tag, prop);
}

void SettingsDialog::delTag()
{
	QTableWidgetItem* item = ui->tagView->currentItem();
	if (!item)
		return;
	QString tag = item->text();
	mDatabase->deleteDbTag(tag);
}


PropertySettingsModel::PropertySettingsModel(TracksModel *db, QWidget *parent):
	QAbstractItemModel(parent), mDatabase(db), mUpdating(false)
{
}

QModelIndex PropertySettingsModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex PropertySettingsModel::parent(const QModelIndex &/*child*/) const
{
	return QModelIndex();
}

int PropertySettingsModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return mPropList.size();
}

int PropertySettingsModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return numberOfColumns;
}

QVariant PropertySettingsModel::data(const QModelIndex &index, int role) const
{
	if (index.parent().isValid())
		return QVariant();
	if (index.row() < 0 || index.row() >= mPropList.size())
		return QVariant();
	Property* prop = mDatabase->propertyType(mPropList[index.row()]);
	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case columnName:
			if (!prop)
				return tr("Tags");
			else
				return prop->name;
		case columnType:
			if (!prop) return tr("Tags");
			switch (prop->type) {
			case Property::Ordinary: return tr("Property");
			case Property::Category: return tr("Checklist");
			case Property::Big: return tr("Text");
			default:;
			}
		}
		break;
	case Qt::CheckStateRole:
	default:;
	}
	return QVariant();
}

bool PropertySettingsModel::setData(const QModelIndex &/*index*/, const QVariant &/*value*/, int /*role*/)
{
	// TODO do hidden
	return false;
}

QVariant PropertySettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();
	if (role != Qt::DisplayRole)
		return QVariant();
	switch(section) {
	case columnName: return tr("Name");
	case columnType: return tr("Type");
	case columnHidden: return tr("Hidden");
	default:;
	}
	return QVariant();
}

void PropertySettingsModel::dbUpdated()
{
	if (mUpdating) return;
	beginResetModel();
	mPropList.clear();
	mPropList.append(tr("Tags"));
	mPropList.append(mDatabase->allProperties());
	mPropList.append(mDatabase->allBigProperties());
	endResetModel();
}

QString PropertySettingsModel::propForIndex(const QModelIndex &index) const
{
	if (index.parent().isValid())
		return QString();
	if (index.row() < 0 || index.row() >= mPropList.size())
		return QString();
	if (index.row() == 0)
		return QString();
	return mPropList[index.row()];
}

QModelIndex PropertySettingsModel::indexForProp(QString prop) const
{
	int ind = mPropList.indexOf(prop);
	if (ind < 0)
		return QModelIndex();
	else
		return index(ind, 0);
}

Qt::ItemFlags PropertySettingsModel::flags(const QModelIndex &index) const
{
	if (!index.parent().isValid() && index.column() == columnHidden)
		return
			Qt::ItemIsEnabled |
			Qt::ItemIsEditable |
			Qt::ItemIsSelectable |
			Qt::ItemIsUserCheckable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
