#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>

#include "tracksmodel.h"
#include "filesmodel.h"
#include "propertymodel.h"
#include "tagsmodel.h"
#include "filtermodel.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mSelected(NULL)
{
	ui->setupUi(this);
	connect(ui->findButton, SIGNAL(clicked(bool)), this, SLOT(onLeftButton()));
	connect(ui->infoButton, SIGNAL(clicked(bool)), this, SLOT(onRightButton()));
	connect(ui->filterTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(onFilterSelected()));
	connect(ui->filterComboBox, SIGNAL(currentTextChanged(QString)),
		this, SLOT(onFilterChanged()));
	connect(ui->filterLineEdit, SIGNAL(textChanged(QString)),
		this, SLOT(onFilterChanged()));

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName("database.sqlite");
	db.open();

	mProgress = new QProgressDialog(this);

	mTracks = new TracksModel(this);
	mFiles = new FilesModel(mTracks, this);
	mProperties = new PropertyModel(mTracks, this);
	mTags = new TagsModel(mTracks, this);
	mFilter = new FilterModel(this);
	mFilter->setSourceModel(mTracks);

	connect(mTracks, SIGNAL(progressStart(int,QString)),
		this, SLOT(onProgressInit(int,QString)));
	connect(mTracks, SIGNAL(progress(int)), mProgress, SLOT(setValue(int)));
	connect(mTracks, SIGNAL(progressEnd()), mProgress, SLOT(reset()));

	ui->dbView->setModel(mFilter);
	connect(ui->dbView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		this, SLOT(onTrackSelected(QModelIndex)));

	connect(mTracks, SIGNAL(recordSelected(Record*)), this, SLOT(onSelected(Record*)));
	connect(mTracks, SIGNAL(dbChanged()), this, SLOT(onDBChanged()));
	connect(mTracks, SIGNAL(tagsChanged()), this, SLOT(onTagsChanged()));

	connect(ui->saveButton, SIGNAL(clicked(bool)), mTracks, SLOT(saveRecord()));
	connect(ui->playButton, SIGNAL(clicked(bool)), this, SLOT(onPlayPlus()));
	connect(ui->playMinusButton, SIGNAL(clicked(bool)), this, SLOT(onPlayMinus()));
	connect(ui->addTagButton, SIGNAL(clicked(bool)), this, SLOT(onAddTag()));
	connect(ui->removeTagButton, SIGNAL(clicked(bool)), this, SLOT(onRemoveTag()));

	ui->filesView->setModel(mFiles);
	ui->propertyView->setModel(mProperties);
	ui->tagsView->setModel(mTags);
	load();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::save()
{
	// TODO
}

void MainWindow::load()
{
	// TODO
	mStoragePath = "/home/baldis/Projects/personal/music";

	mTracks->loadDB();
	mTracks->scanPath(mStoragePath);
}

void MainWindow::onLeftButton()
{
	if (ui->dbWidget->isHidden()) {
		ui->dbWidget->show();
	} else {
		if (ui->infoWidget->isHidden()) {
			ui->infoWidget->show();
		}
		ui->dbWidget->hide();
	}
}

void MainWindow::onRightButton()
{
	if (ui->infoWidget->isHidden()) {
		ui->infoWidget->show();
	} else {
		if (ui->dbWidget->isHidden()) {
			ui->dbWidget->show();
		}
		ui->infoWidget->hide();
	}
}

void MainWindow::onSelected(Record* rec)
{
	mSelected = rec;
	QStringList bigprops = mBigProps.keys();
	if (!mSelected) {
		foreach (QString prop, bigprops)
			mBigProps[prop]->clear();
		return;
	}
	foreach (QString prop, bigprops)
		mBigProps[prop]->setPlainText(
					mSelected->property(prop).toString());
}

void MainWindow::onFilterSelected()
{
	switch(ui->filterTypeComboBox->currentIndex()) {
	case filterTitle:
	case filterArtist:
	case filterAlbum:
		ui->filterLineEdit->setVisible(true);
		ui->filterComboBox->setVisible(false);
		//ui->filterLineEdit->clear();
		break;
	case filterTags:
		ui->filterLineEdit->setVisible(false);
		ui->filterComboBox->setVisible(true);
		//ui->filterLineEdit->clear();
		ui->filterComboBox->clear();
		ui->filterComboBox->addItems(mTracks->allTags());
		break;
	default: {
		Property* prop = mTracks->propertyType(ui->filterTypeComboBox->currentText());
		if (!prop)
			return;
		ui->filterLineEdit->setVisible(prop->type != Property::Category);
		ui->filterComboBox->setVisible(prop->type == Property::Category);
		ui->filterComboBox->clear();
		//ui->filterLineEdit->clear();
		ui->filterComboBox->addItems(mTracks->allCategories(prop->name));
	}
	}
	onFilterChanged();
}

void MainWindow::onFilterChanged()
{
	int colNo = -1;
	switch (ui->filterTypeComboBox->currentIndex()) {
	case filterTitle: colNo = TracksModel::trackColumn; break;
	case filterArtist: colNo = TracksModel::artistColumn; break;
	case filterAlbum: colNo = TracksModel::albumColumn; break;
	case filterTags: colNo = TracksModel::tagsColumn; break;
	default:
		colNo =
			TracksModel::columnNumber +
			ui->filterTypeComboBox->currentIndex() -
			filterCustom;
	}
	QString filter =
		ui->filterLineEdit->isVisible() ?
		ui->filterLineEdit->text() :
		ui->filterComboBox->currentText();

	mFilter->setFilter(colNo, filter);
}

void MainWindow::onTrackSelected(QModelIndex index)
{
	mTracks->selectRecord(mFilter->mapToSource(index));
}

void MainWindow::onPlayPlus()
{
	if (!mSelected || mSelected->main().isEmpty())
		return;
	play(mSelected->main());
}

void MainWindow::onPlayMinus()
{
	if (!mSelected || mSelected->minus().isEmpty())
		return;
	play(mSelected->minus());
}

void MainWindow::onAddTag()
{
	if (!mSelected)
		return;
	mTracks->addTag(ui->tagCombo->currentText());
	mTags->onRecordChanged(mSelected);
}

void MainWindow::onRemoveTag()
{
	if (!mSelected)
		return;
	QModelIndex index = ui->tagsView->currentIndex();
	QString tag = mTags->tagForIndex(index);
	if (!tag.isEmpty())
		mTracks->removeTag(tag);
	mTags->onRecordChanged(mSelected);
}

void MainWindow::onBigPropertyEdited()
{
	if (!mSelected)
		return;
	QString prop = sender()->property("propname").toString();
	if (prop.isEmpty())
		return;
	mSelected->setProperty(prop, mBigProps[prop]->toPlainText());
}

void MainWindow::play(QString file)
{
	QUrl url = QUrl::fromLocalFile(file);
	QDesktopServices::openUrl(url);
	File* f = mTracks->fileForName(file);
	if (f) f->lastPlayed = QDateTime::currentDateTime();
}

void MainWindow::onProgressInit(int range, QString label)
{
	mProgress->reset();
	mProgress->setMinimumDuration(1000);
	mProgress->setRange(0, range);
	mProgress->setLabelText(label);
}

void MainWindow::onDBChanged()
{
	QStringList bigProps = mTracks->allBigProperties();
	while (ui->tabWidget->count() > 1) {
		QWidget* page = ui->tabWidget->widget(1);
		ui->tabWidget->removeTab(1);
		delete page;
	}
	mBigProps.clear();

	foreach(QString prop, bigProps) {
		QWidget* page = new QWidget(ui->tabWidget);
		QVBoxLayout* layout = new QVBoxLayout(page);
		page->setLayout(layout);
		QPlainTextEdit* editor = new QPlainTextEdit(page);
		layout->addWidget(editor);
		ui->tabWidget->addTab(page, prop);
		mBigProps[prop] = editor;
		editor->setProperty("propname", prop);
		connect(editor, SIGNAL(textChanged()), this,
			SLOT(onBigPropertyEdited()));
	}

	QStringList filters = mTracks->allProperties() + mTracks->allBigProperties();
	filters.prepend(tr("Tags"));
	filters.prepend(tr("Album"));
	filters.prepend(tr("Artist"));
	filters.prepend(tr("Title"));
	ui->filterTypeComboBox->clear();
	ui->filterTypeComboBox->addItems(filters);
}

void MainWindow::onTagsChanged()
{
	ui->tagCombo->clear();
	ui->tagCombo->addItems(mTracks->allTags());
}
