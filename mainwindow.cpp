#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include "tracksmodel.h"
#include "filesmodel.h"
#include "propertymodel.h"
#include "tagsmodel.h"
#include "filtermodel.h"
#include "combocheckboxdelegate.h"
#include "printthread.h"
#include "printselectdialog.h"
#include "settingsdialog.h"

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
	mProgress->setWindowTitle(tr("Resistance is futile"));

	mPrinter = new PrintHelper(this);
	mPrintDialog = new PrintSelectDialog(this);

	mTracks = new TracksModel(this);
	mFiles = new FilesModel(mTracks, this);
	mProperties = new PropertyModel(mTracks, this);
	mTags = new TagsModel(mTracks, this);
	mFilter = new FilterModel(this);
	mFilter->setSourceModel(mTracks);

	mSettings = new SettingsDialog(mTracks, this);

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
	connect(mTracks, SIGNAL(recordChanged()), this, SLOT(onRecordChanged()));
	connect(mTracks, SIGNAL(recordSelected(Record*)), this, SLOT(onRecordChanged()));

	connect(ui->filesView, SIGNAL(doubleClicked(QModelIndex)),
		this, SLOT(onFileActivated(QModelIndex)));

	connect(ui->actionSaveTrack, SIGNAL(triggered(bool)), mTracks, SLOT(saveRecord()));
	connect(ui->actionNewTrack, SIGNAL(triggered(bool)), mTracks, SLOT(newRecord()));
	connect(ui->actionCancelChanges, SIGNAL(triggered(bool)), mTracks, SLOT(revertRecord()));

	connect(ui->actionPlayTrack, SIGNAL(triggered(bool)), this, SLOT(onPlayPlus()));
	connect(ui->actionPlayTrackMinus, SIGNAL(triggered(bool)), this, SLOT(onPlayMinus()));
	connect(ui->actionPrint, SIGNAL(triggered(bool)), this, SLOT(onPrint()));

	connect(ui->actionAddFolder, SIGNAL(triggered(bool)), this, SLOT(onSetStorage()));
	connect(ui->actionExport, SIGNAL(triggered(bool)), this, SLOT(onExport()));
	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
	connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(onSettings()));

	connect(ui->actionFilterFilesOnly, SIGNAL(toggled(bool)), this, SLOT(onFFilter(bool)));
	connect(ui->actionFilterFullInfo, SIGNAL(toggled(bool)), this, SLOT(onIFFilter(bool)));
	connect(ui->actionFilterInfoOnly, SIGNAL(toggled(bool)), this, SLOT(onIFilter(bool)));

	connect(ui->addTagButton, SIGNAL(clicked(bool)), this, SLOT(onAddTag()));
	connect(ui->removeTagButton, SIGNAL(clicked(bool)), this, SLOT(onRemoveTag()));

	connect(ui->addFileButton, SIGNAL(clicked(bool)), this, SLOT(onAddFile()));
	connect(ui->deleteFileButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteFile()));
	connect(ui->moveFileButton, SIGNAL(clicked(bool)), this, SLOT(onMoveFile()));
	connect(ui->autoFileButton, SIGNAL(clicked(bool)), this, SLOT(onAutoFile()));
	// no logic yet
	ui->deleteFileButton->setVisible(false);
	ui->moveFileButton->setVisible(false);

	ui->filesView->setModel(mFiles);
	ui->propertyView->setModel(mProperties);
	ui->propertyView->setItemDelegateForColumn(
		PropertyModel::valueColumn,
		new ComboCheckBoxDelegate(this, ", "));
	ui->tagsView->setModel(mTags);
	load();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::save()
{
	QSettings settings;

	settings.beginGroup("GUI");
	settings.setValue("mwgeom", saveGeometry());
	settings.setValue("mwstate", saveState());
	settings.setValue("tracktable", ui->dbView->horizontalHeader()->saveState());
	settings.setValue("proptable", ui->propertyView->horizontalHeader()->saveState());
	settings.setValue("filestable", ui->filesView->horizontalHeader()->saveState());
	settings.setValue("tagstable", ui->tagsView->horizontalHeader()->saveState());
	settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("dbshown", ui->dbWidget->isVisible());
	settings.setValue("infoshown", ui->infoWidget->isVisible());
	settings.endGroup();

	settings.beginGroup("FILTER");
	settings.setValue("fullfilter", ui->actionFilterFullInfo->isChecked());
	settings.setValue("filefilter", ui->actionFilterFilesOnly->isChecked());
	settings.setValue("infofilter", ui->actionFilterInfoOnly->isChecked());
	settings.endGroup();

	settings.beginGroup("PATHS");
	settings.setValue("storage", mStoragePath);
	settings.setValue("lastadd", mLastOpenPath);
	settings.setValue("lastimp", mLastExpImpPath);
	settings.endGroup();

	settings.beginGroup("PRINT");
	settings.setValue("checked", mPrintSelected);
	settings.endGroup();
}

void MainWindow::load()
{
	QSettings settings;

	settings.beginGroup("GUI");
	restoreGeometry(settings.value("mwgeom").toByteArray());
	restoreState(settings.value("mwstate").toByteArray());
	ui->dbView->horizontalHeader()->restoreState(settings.value("tracktable").toByteArray());
	ui->propertyView->horizontalHeader()->restoreState(settings.value("proptable").toByteArray());
	ui->filesView->horizontalHeader()->restoreState(settings.value("filestable").toByteArray());
	ui->tagsView->horizontalHeader()->restoreState(settings.value("tagstable").toByteArray());
	ui->splitter->restoreState(settings.value("splitter").toByteArray());
	ui->dbWidget->setVisible(settings.value("dbshown", true).toBool());
	ui->infoWidget->setVisible(settings.value("infoshown", true).toBool());
	settings.endGroup();

	settings.beginGroup("FILTER");
	ui->actionFilterFullInfo->setChecked(settings.value("fullfilter", true).toBool());
	ui->actionFilterFilesOnly->setChecked(settings.value("filefilter", false).toBool());
	ui->actionFilterInfoOnly->setChecked(settings.value("infofilter", true).toBool());
	settings.endGroup();

	settings.beginGroup("PATHS");
	mStoragePath = settings.value("storage").toString();
	mLastOpenPath = settings.value("lastadd").toString();
	mLastExpImpPath = settings.value("lastimp").toString();
	settings.endGroup();

	settings.beginGroup("PRINT");
	mPrintSelected = settings.value("checked").toStringList();
	settings.endGroup();

	if (mStoragePath.isEmpty()) {
		QMessageBox::information(
			this, tr("Use the force, Luke"),
			tr("Select a folder for storage of music files"));
		onSetStorage();
	}

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
		mBigProps[prop]->setHtml(
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

void MainWindow::onFileActivated(QModelIndex index)
{
	QString file = mFiles->fileForIndex(index);
	if (!file.isEmpty())
		play(file);
}

void MainWindow::onPlayPlus()
{
	if (!mSelected || mSelected->prime().isEmpty())
		return;
	play(mSelected->prime());
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

void MainWindow::onAddFile()
{
	if (!mSelected)
		return;
	QStringList res = QFileDialog::getOpenFileNames(
		this,
		tr("Select one or more files to add"),
		mLastOpenPath,
		QString(tr("Tracks (%1)")).arg(SUPPORTED_TYPES));
	QStringList files = res;
	foreach(QString file, files) {
		QFileInfo fi(file);
		mLastOpenPath = fi.absolutePath();
		if (!fi.absolutePath().toLower().startsWith(mStoragePath.toLower())) {
			// not in a subdir
			QDir(mStoragePath).mkpath("Auto");
			QString newName = QString("%1/Auto/%2").
					arg(mStoragePath).arg(fi.fileName());
			QFile::copy(file, newName);
			mTracks->addFile(newName);
		} else
			mTracks->addFile(file);
	}
}

void MainWindow::onMoveFile()
{
	if (!mSelected)
		return;
	QStringList files;
	// TODO
	foreach(QString file, files)
		mTracks->addFile(file);
}

void MainWindow::onDeleteFile()
{
	if (!mSelected)
		return;
	// TODO
}

void MainWindow::onAutoFile()
{
	if (!mSelected)
		return;
	QStringList files = mTracks->allFiles();
	foreach(QString file, files) {
		File* f = mTracks->fileForName(file);
		if (f && mSelected->match(f))
			mTracks->addFile(file);
	}
}

void MainWindow::onSetStorage()
{
	if (!mStoragePath.isEmpty()) {
		if (QMessageBox::warning(
			this, tr("All your base are belong to us"),
			tr("All old files will be removed from database! Continue?"),
			QMessageBox::Ok | QMessageBox::Cancel) ==
				QMessageBox::Cancel)
			return;
	}
	QString newStorage = QFileDialog::getExistingDirectory(
		this, tr("Select new storage"), mStoragePath);
	if (!newStorage.isEmpty()) {
		mTracks->clearFiles();
		mTracks->scanPath(mStoragePath = newStorage);
	}
}

void MainWindow::onImport()
{
	// TODO
}

void MainWindow::onExport()
{
	// TODO
}

void MainWindow::onPrint()
{
	if (!mSelected) return;
	mPrintDialog->setSelected(mPrintSelected);
	if (mPrintDialog->exec()) {
		QStringList docs;
		mPrintSelected = mPrintDialog->selected();
		foreach(QString prop, mPrintSelected)
			docs.append(mTracks->printHtml(prop));
		mPrinter->print(docs);
	}
}

void MainWindow::onSettings()
{
	mSettings->show();
}

void MainWindow::onIFilter(bool on)
{
	mFilter->setStateFilter(noFileState, on);
}

void MainWindow::onFFilter(bool on)
{
	mFilter->setStateFilter(noInfoState, on);
	mFilter->setStateFilter(oneFileState, on);
}

void MainWindow::onIFFilter(bool on)
{
	mFilter->setStateFilter(noMinusState, on);
	mFilter->setStateFilter(fullState, on);
}

void MainWindow::onBigPropertyEdited()
{
	if (!mSelected)
		return;
	QString prop = sender()->property("propname").toString();
	if (prop.isEmpty())
		return;
	if (mSelected->setProperty(prop, mBigProps[prop]->toHtml()))
		mTracks->updateRecord();
}

void MainWindow::play(QString file)
{
	QUrl url = QUrl::fromLocalFile(file);
	if (!QDesktopServices::openUrl(url))
		QMessageBox::critical(this, tr("He is dead, Jim"),
			tr("There was an error\n"
			   "Probably there's no appropriate application"));
	mTracks->playFile(file);
}

void MainWindow::onProgressInit(int range, QString label)
{
	mProgress->reset();
	mProgress->setMinimumDuration(1000);
	mProgress->setRange(0, range);
	mProgress->setLabelText(label);
	mProgress->setValue(0);
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
		QTextEdit* editor = new QTextEdit(page);
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

	mPrintDialog->clear();
	foreach(QString prop, mTracks->allBigProperties())
		mPrintDialog->addOption(prop);
}

void MainWindow::onTagsChanged()
{
	ui->tagCombo->clear();
	ui->tagCombo->addItems(mTracks->allTags());
}

void MainWindow::onRecordChanged()
{
	ui->actionCancelChanges->setEnabled(mSelected && mSelected->changed());
	ui->actionSaveTrack->setEnabled(mSelected && mSelected->changed());
	ui->actionPlayTrack->setEnabled(mSelected && !mSelected->prime().isEmpty());
	ui->actionPlayTrackMinus->setEnabled(mSelected && !mSelected->minus().isEmpty());
	ui->actionPrint->setEnabled(mSelected);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	save();
	QMainWindow::closeEvent(e);
}
