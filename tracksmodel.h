#ifndef TRACKSMODEL_H
#define TRACKSMODEL_H

#include <QAbstractItemModel>

#include "database.h"

#define SUPPORTED_TYPES "*.mp3"

class TracksModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum Columns {
		stateColumn = 0,
		idColumn,
		lastPlayedColumn,
		trackColumn,
		artistColumn,
		albumColumn,
		tagsColumn,

		columnNumber
	};

	TracksModel(QObject* parent = NULL);

	// QAbstractItemModel interface
	virtual QModelIndex index(
		int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(
		const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(
		const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(
		int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	bool isValidIndex(const QModelIndex &index) const;

	// database
	Record* recordForIndex(const QModelIndex& index) const;
	Record* selectedRecord() const;

	// for selected record
	bool setMainFile(QString file);
	bool setMinusFile(QString file);
	bool addFile(QString file);
	bool removeFile(QString file);
	File* fileForName(QString file);

	bool addTag(QString tag);	// both new and existing
	bool removeTag(QString tag);

	QString printHtml(QString property = QString()) const;

	// for tables
	QStringList allTags() const;
	QStringList allCategories(QString type) const;
	QStringList allProperties() const;
	QStringList allBigProperties() const;
	QStringList allFiles() const;

	Property* propertyType(QString prop) const;

public slots:
	void loadDB();
	void clearFiles();
	void scanPath(const QString& path);

	void selectRecord(const QModelIndex& index);
	void saveRecord();
	void revertRecord();
	void newRecord();

	void playFile(const QString& file);

	void updateRecord();

signals:
	void message(QString msg);
	void progressStart(int size, QString msg);
	void progressEnd();
	void progress(int p);

	void recordSelected(Record* rec);
	void recordChanged();
	void dbChanged();
	void tagsChanged();

protected:
	File* readFile(QString filename);

	int insertTrack();
	void updateTrack(Record* rec, Record *origin);

	void fillFile(File *file);

private:
	QMap<int, Record*> mTracks;
	QMap<int, File*> mFiles;
	QMap<QString, Property*> mProperties;
	QMap<QString, Tag*> mTags;

	QList<int> mTrackIds;
	Record* mSelectedTrack;
	QMap<QString, int> mFileIds;
	QStringList mCatProps;
	QStringList mOrdProps;
	QStringList mBigProps;
	QMap<QString, QStringList> mCatTags;
};

#endif // TRACKSMODEL_H
