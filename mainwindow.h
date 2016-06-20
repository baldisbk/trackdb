#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QPlainTextEdit>
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
	void onBigPropertyEdited();

	// action slots
	void onPlayPlus();
	void onPlayMinus();
	void onAddTag();
	void onRemoveTag();
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

private:
	Ui::MainWindow *ui;

	Record* mSelected;

	QString mStoragePath;

	QProgressDialog* mProgress;
	TracksModel* mTracks;
	FilesModel* mFiles;
	PropertyModel* mProperties;
	TagsModel* mTags;
	FilterModel* mFilter;

	QMap<QString, QPlainTextEdit*> mBigProps;
};

#endif // MAINWINDOW_H
