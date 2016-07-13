#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QTextEdit>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class TracksModel;
class FilesModel;
class PropertyModel;
class TagsModel;
class FilterModel;
class Record;
class PrintHelper;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	enum FilterType {
		filterTitle = 0,
		filterArtist,
		filterAlbum,
		filterTags,

		filterCustom
	};

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *e);

private:
	void save();
	void load();

private slots:
	void onLeftButton();
	void onRightButton();
	void onSelected(Record *rec);
	void onFilterSelected();
	void onFilterChanged();
	void onTrackSelected(QModelIndex index);
	void onFileActivated(QModelIndex index);
	void onBigPropertyEdited();

	// action slots
	void onPlayPlus();
	void onPlayMinus();
	void onAddTag();
	void onRemoveTag();
	void onAddFile();
	void onMoveFile();
	void onDeleteFile();
	void onAutoFile();
	void onSetStorage();
	void onImport();
	void onExport();
	void onPrint();

	void onIFilter(bool on);
	void onFFilter(bool on);
	void onIFFilter(bool on);

	void play(QString file);

	void onProgressInit(int range, QString label);

	void onDBChanged();
	void onTagsChanged();
	void onRecordChanged();

private:
	Ui::MainWindow *ui;

	Record* mSelected;

	// settings
	QString mStoragePath;
	QString mLastOpenPath;
	QString mLastExpImpPath;

	QProgressDialog* mProgress;
	TracksModel* mTracks;
	FilesModel* mFiles;
	PropertyModel* mProperties;
	TagsModel* mTags;
	FilterModel* mFilter;

	QMap<QString, QTextEdit*> mBigProps;

	PrintHelper* mPrinter;
};

#endif // MAINWINDOW_H
