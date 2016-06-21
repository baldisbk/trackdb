#include "tracksmodel.h"

#include <QColor>
#include <QDir>
#include <QIcon>

#include <QSqlQuery>
#include <QSqlRecord>

#include <QProcess>

#include <QDebug>

TracksModel::TracksModel(QObject *parent):
	QAbstractItemModel(parent), mSelectedTrack(NULL)
{
}

QModelIndex TracksModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex TracksModel::parent(const QModelIndex &/*child*/) const
{
	return QModelIndex();
}

int TracksModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return mTrackIds.size();
}

int TracksModel::columnCount(const QModelIndex &/*parent*/) const
{
	return columnNumber + mCatProps.size() + mOrdProps.size();
}

QVariant TracksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical)
		return QVariant();
	if (role != Qt::DisplayRole)
		return QVariant();
	switch (section) {
	case idColumn: return tr("#");
	case trackColumn: return tr("Track");
	case artistColumn: return tr("Artist");
	case albumColumn: return tr("Album");
	case lastPlayedColumn: return tr("Last played");
	case tagsColumn: return tr("Tags");
	default:
		if (section < columnNumber)
			break;
		if (section < columnNumber + mOrdProps.size())
			return mOrdProps[section - columnNumber];
		if (section < columnCount())
			return mCatProps[section - columnNumber - mOrdProps.size()];
		break;
	}
	return QVariant();
}

bool TracksModel::isValidIndex(const QModelIndex &index) const
{
	return	!index.parent().isValid() &&
		index.column() >= 0 && index.column() < columnCount() &&
		index.row() >= 0 && index.row() < rowCount();
}

Record *TracksModel::recordForIndex(const QModelIndex &index) const
{
	if (isValidIndex(index))
		return mTracks.value(mTrackIds[index.row()], NULL);
	else
		return NULL;
}

Record *TracksModel::selectedRecord() const
{
	return mSelectedTrack;
}

bool TracksModel::setMainFile(QString file)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->files().contains(file))
		if (!addFile(file))
			return false;
	if (mSelectedTrack->minus() == file)
		mSelectedTrack->mMinus = mSelectedTrack->main();
	mSelectedTrack->mMain = file;
	return true;
}

bool TracksModel::setMinusFile(QString file)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->files().contains(file))
		if (!addFile(file))
			return false;
	if (mSelectedTrack->main() == file)
		mSelectedTrack->mMain = mSelectedTrack->minus();
	mSelectedTrack->mMinus = file;
	return true;
}

bool TracksModel::addFile(QString file)
{
	if (!mSelectedTrack)
		return false;
	if (mSelectedTrack->files().contains(file))
		return false;
	if (!mFileIds.contains(file)) {
		File* f = readFile(file);
		mSelectedTrack->addFile(f);
	} else {
		File* f = mFiles[mFileIds[file]];
		// TODO do this in saveRecord()
		// and setting proper trackId too!
		//if (f->track() >= 0)
		//	mTracks[f->track()]->removeFile(f);
		mSelectedTrack->addFile(f);
	}
	return true;
}

bool TracksModel::removeFile(QString file)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->files().contains(file))
		return false;
	File* f = mFiles[mFileIds[file]];
	mSelectedTrack->removeFile(f);
	if (mSelectedTrack->fill() == emptyState) {
		// no need to clear files, 'cause there's none
		Record* toRemove = mSelectedTrack;
		int index = mTrackIds.indexOf(mSelectedTrack->id());
		selectRecord(QModelIndex());

		beginRemoveRows(QModelIndex(), index, index);
		mTrackIds.removeAll(toRemove->id());
		mTracks.remove(toRemove->id());
		endRemoveRows();

		delete toRemove;
	}
	return true;
}

File *TracksModel::fileForName(QString file)
{
	if (mFileIds.contains(file))
		return mFiles[mFileIds[file]];
	else
		return NULL;
}

bool TracksModel::addTag(QString tag)
{
	if (!mSelectedTrack)
		return false;
	if (mTags.contains(tag)) {
		if (!allTags().contains(tag))
			return false;
	} else {
		Tag* t = new Tag;
		t->name = tag;
		// TODO SQL insert to database
		mTags[tag] = t;
		mCatTags[QString()].append(tag);
		emit tagsChanged();
	}
	if (mSelectedTrack->tags().contains(tag))
		return false;
	mSelectedTrack->mTags.append(tag);
	return true;
}

bool TracksModel::removeTag(QString tag)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->tags().contains(tag))
		return false;
	mSelectedTrack->mTags.removeAll(tag);
	return true;
}

QStringList TracksModel::allTags() const
{
	return mCatTags.value(QString());
}

QStringList TracksModel::allCategories(QString type) const
{
	return mCatTags.value(type);
}

QStringList TracksModel::allProperties() const
{
	return mOrdProps + mCatProps;
}

QStringList TracksModel::allBigProperties() const
{
	return mBigProps;
}

Property *TracksModel::propertyType(QString prop) const
{
	return mProperties.value(prop, NULL);
}

QVariant TracksModel::data(const QModelIndex &index, int role) const
{
	if (!isValidIndex(index))
		return QVariant();
	Record* rec = recordForIndex(index);
	switch (role) {
	case Qt::UserRole:
		if (index.column() == stateColumn)
			return int(rec->fill());
		break;
	case Qt::DisplayRole:
		switch (index.column()) {
		case idColumn: if (rec->id() > 0) return rec->id(); else return "---";
		case lastPlayedColumn: {
			QDateTime last;
			foreach(QString file, rec->files()) {
				File* f = mFiles[mFileIds[file]];
				if (!f->lastPlayed.isNull())
					if (last.isNull() || last > f->lastPlayed)
						last = f->lastPlayed;
			}
			if (last.isNull())
				return tr("Never");
			else
				return last.toString("ddd d MMM yyyy HH:mm");
		}
		case trackColumn: return rec->title;
		case artistColumn: return rec->artist;
		case albumColumn: return rec->album;
		case tagsColumn: return rec->tags().join(", ");
		default: {
			if (index.column() < columnNumber)
				break;
			QVariant prop;
			if (index.column() < columnNumber + mOrdProps.size())
				prop = rec->property(
					mOrdProps[index.column() - columnNumber]);
			else
				prop = rec->property(
					mCatProps[index.column() - columnNumber - mOrdProps.size()]);
			if (prop.type() == QVariant::String)
				return prop;
			else if (prop.type() == QVariant::StringList)
				return prop.toStringList().join(", ");
			else
				return QVariant();
		}
		}
		break;
	case Qt::ForegroundRole:
		switch (rec->fill()) {
		case noInfoState:
		case oneFileState:
			return QColor(Qt::red);
		case noFileState:
			return QColor(Qt::lightGray);
		case noMinusState:
			return QColor(Qt::darkGray);
		default:;
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == stateColumn)
			switch (rec->fill()) {
			case fullState: return QIcon(":/pics/hasall.png");
			case noInfoState: return QIcon(":/pics/noinfo.png");
			case noFileState: return QIcon(":/pics/hasinfo.png");
			case noMinusState: return QIcon(":/pics/nominus.png");
			case oneFileState: return QIcon(":/pics/hastrack.png");
			default:;
			}
		break;
	case Qt::SizeHintRole:
		switch (index.column()) {
		case idColumn:
		case stateColumn:
			return QSize(1, 1);
		default:
			return QVariant();
		}
	default:;
	}
	return QVariant();
}

QStringList scanDir(const QString &path)
{
	QDir dir(path);
	QStringList filters;
	filters << "*.mp3" << "*.jpeg" << "*.png";
	QStringList res;
	QStringList files = dir.entryList(filters, QDir::Files);
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString fn, files) res += path + "/" + fn;
	foreach (QString d, dirs) res += scanDir(path + "/" + d);
	return res;
}

void TracksModel::loadDB()
{
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery qSelectProps(db);
	qSelectProps.exec("SELECT id, name, big, fixed FROM property");
	while(qSelectProps.next()) {
		Property* prop = new Property;
		prop->mId = qSelectProps.value(0).toInt();
		prop->name = qSelectProps.value(1).toString();
		if (qSelectProps.value(2).toBool()) {
			prop->type = Property::Big;
			mBigProps.append(prop->name);
		} else if (qSelectProps.value(3).toBool()) {
			prop->type = Property::Category;
			mCatProps.append(prop->name);
		} else {
			prop->type = Property::Ordinary;
			mOrdProps.append(prop->name);
		}
		mProperties[prop->name] = prop;
	}
	emit dbChanged();

	QSqlQuery qSelectTags(db);
	qSelectTags.exec(
		"SELECT tag.id, tag, property.name "
		"FROM tag LEFT JOIN property ON tag.property=property.id");
	while(qSelectTags.next()) {
		Tag* tag = new Tag;
		tag->mId = qSelectTags.value(0).toInt();
		tag->name = qSelectTags.value(1).toString();
		tag->type = qSelectTags.value(2).toString();
		mTags[tag->name] = tag;
		mCatTags[tag->type].append(tag->name);
	}
	emit tagsChanged();


	QSqlQuery qSelectTracks(db);
	qSelectTracks.exec(
		"SELECT id, title, artist, album, lastchange FROM track");
	beginResetModel();
	while(qSelectTracks.next()) {
		Record* trc = new Record;
		trc->mId = qSelectTracks.value(0).toInt();
		trc->title = qSelectTracks.value(1).toString();
		trc->artist = qSelectTracks.value(2).toString();
		trc->album = qSelectTracks.value(3).toString();
		trc->lastChanged = qSelectTracks.value(4).toDateTime();
		trc->mFill = noFileState;
		mTracks[trc->id()] = trc;
		mTrackIds.append(trc->id());
	}

	QSqlQuery qSelectFiles(db);
	qSelectFiles.exec(
		"SELECT file.id, file.title, file.artist, file.album, "
		"	filename, track, lastplayed, main, minus "
		"FROM file "
		"LEFT JOIN track ON track.id=file.track");
	while(qSelectFiles.next()) {
		File* file = new File;
		file->mId = qSelectFiles.value(0).toInt();
		file->title = qSelectFiles.value(1).toString();
		file->artist = qSelectFiles.value(2).toString();
		file->album = qSelectFiles.value(3).toString();
		file->filename = qSelectFiles.value(4).toString();
		file->mTrack = qSelectFiles.value(5).toInt();
		file->lastPlayed = qSelectFiles.value(6).toDateTime();

		Record* rec = mTracks.value(file->track(), NULL);
		if (!rec) {
			bool found = false;
			foreach(Record* r, mTracks) {
				if (r->addFile(file)) {
					found = true;
					file->mTrack = r->id();
					break;
				}
			}
			if (!found) {
				rec = new Record(file, -mTracks.size() - 1);
				mTracks[rec->id()] = rec;
				mTrackIds.append(rec->id());
			}
		} else {
			bool nomain = rec->main().isEmpty();
			bool nominus = rec->minus().isEmpty();
			rec->addFile(file);
			if (file->track() == qSelectFiles.value(7).toInt())
				rec->mMain = file->filename;
			else if (nomain)
				rec->mMain.clear();
			if (file->track() == qSelectFiles.value(8).toInt())
				rec->mMinus = file->filename;
			else if (nominus)
				rec->mMinus.clear();
		}
		mFileIds[file->filename] = file->id();
		mFiles[file->id()] = file;
	}


	QSqlQuery qSelectTagForTrack(db);
	qSelectTagForTrack.exec(
		"SELECT track.id, tag.tag "
		"FROM track "
		"JOIN tracktag ON tracktag.track=track.id "
		"JOIN tag ON tracktag.tag=tag.id "
		"WHERE tag.property=0");
	while(qSelectTagForTrack.next()) {
		Record *rec = mTracks.value(qSelectTagForTrack.value(0).toInt(), NULL);
		if (!rec)
			continue;
		rec->mTags.append(qSelectTagForTrack.value(1).toString());
	}

	QSqlQuery qSelectPropForTracks(db);
	// zats wan grit kwery!!! B-)
	qSelectPropForTracks.exec(
		"SELECT id, name, val FROM ("
		"	SELECT track.id, name, value as val "
		"	FROM track "
		"	JOIN trackproperty ON track.id=trackproperty.track "
		"	JOIN property ON trackproperty.property=property.id "
		"UNION "
		"	SELECT track.id, name, tag.tag as val "
		"	FROM track "
		"	JOIN tracktag ON tracktag.track=track.id "
		"	JOIN tag ON tracktag.tag=tag.id "
		"	JOIN property ON tag.property=property.id"
		")"
		);
	while(qSelectPropForTracks.next()) {
		Record *rec = mTracks.value(qSelectPropForTracks.value(0).toInt(), NULL);
		if (!rec)
			continue;
		rec->mProperties[qSelectPropForTracks.value(1).toString()] =
			qSelectPropForTracks.value(2).toString();
	}

	endResetModel();
}

void TracksModel::scanPath(const QString &path)
{
	QStringList files = scanDir(path);
	emit progressStart(files.size(), tr("Scanning new tracks"));
	int number = 0;
	beginResetModel();

	foreach(QString file, files) {

		if (mFileIds.contains(file)) {
			emit progress(number++);
			continue;
		}

		File* newFile = readFile(file);
		if (!newFile) {
			emit progress(number++);
			continue;
		}

		// find appropriate record
		bool found = false;
		foreach(Record* rec, mTracks) {
			if (rec->addFile(newFile)) {
				found = true;
				newFile->mTrack = rec->id();
				break;
			}
		}
		if (!found) {
			Record* rec = new Record(newFile, -mTracks.size() - 1);
			mTracks[rec->id()] = rec;
			mTrackIds.append(rec->id());
		}

		emit progress(number++);
	}
	emit progressEnd();
	endResetModel();
}

void TracksModel::selectRecord(const QModelIndex &index)
{
	// TODO ask for changes
	Record* newrec = new Record;
	Record* origin = recordForIndex(index);
	*newrec = *origin;
	emit recordSelected(newrec);
	if (mSelectedTrack)
		delete mSelectedTrack;
	mSelectedTrack = newrec;
}

void TracksModel::saveRecord()
{
	if (!mSelectedTrack)
		return;
	Record* origin = mTracks.value(mSelectedTrack->id(), NULL);
	if (!origin) {
		int id = mTrackIds.size();
		// TODO SQL insert
		// id = lastInsertId
		beginInsertRows(QModelIndex(), mTrackIds.size(), mTrackIds.size());
		Record* newrec = new Record;
		mTracks[id] = newrec;
		mSelectedTrack->mId = id;
		*newrec = *mSelectedTrack;
		mTrackIds.append(id);
		endInsertRows();
	} else {
		if (origin->id() < 0) {
			int id = mTrackIds.size();
			// TODO SQL insert
			// id = lastInsertId
			// TODO change id in mTrackIds, not append/remove
			mTracks.remove(origin->id());
			Record* newrec = new Record;
			mTracks[id] = newrec;
			mSelectedTrack->mId = id;
			*newrec = *mSelectedTrack;
			int ind = mTrackIds.indexOf(origin->id());
			if (ind < 0) {
				beginInsertRows(QModelIndex(), mTrackIds.size(), mTrackIds.size());
				mTrackIds.append(id);
				endInsertRows();
			} else {
				mTrackIds[ind] = id;
				emit dataChanged(index(ind, 0), index(ind, columnCount()-1));
			}
			delete origin;
		} else {
			// TODO SQL update
			int ind = mTrackIds.indexOf(origin->id());
			*origin = *mSelectedTrack;
			emit dataChanged(index(ind, 0), index(ind, columnCount()-1));
		}
	}
}

void TracksModel::revertRecord()
{
	if (!mSelectedTrack)
		return;
	if (!mTracks.contains(mSelectedTrack->id())) {
		delete mSelectedTrack;
		mSelectedTrack = NULL;
	} else {
		Record* origin = mTracks[mSelectedTrack->id()];
		*mSelectedTrack = *origin;
	}
	emit recordSelected(mSelectedTrack);
}

void TracksModel::newRecord()
{
	// TODO ask for changes
	Record* newrec = new Record;
	emit recordSelected(newrec);
	if (mSelectedTrack)
		delete mSelectedTrack;
	mSelectedTrack = newrec;
}

void TracksModel::playFile(const QString &file)
{
	File* f = fileForName(file);
	if (f) {
		f->lastPlayed = QDateTime::currentDateTime();
		// TODO SQL save play time
	}
}

File *TracksModel::readFile(QString filename)
{
	if (mFileIds.contains(filename))
		return mFiles[mFileIds[filename]];

	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery qAddFile(db);
	qAddFile.prepare(
		"INSERT INTO file (filename, title, artist, album) "
		"VALUES (:filename, :title, :artist, :album)");

	QProcess prc;
	prc.start("./id3", QStringList() << filename);
	if (!prc.waitForStarted() || !prc.waitForFinished()) {
		return NULL;
	}

	QByteArray res = prc.readAll();
	QStringList lines = QString::fromLocal8Bit(res).split('\n');

	File* file = new File;
	file->filename = filename;

	foreach(QString line, lines) {
		int split = line.indexOf(":");
		if (split < 0)
			continue;
		QString tagname = line.left(split);
		QString tagvalue = line.remove(0, split+1).simplified();
		if (tagname == "Title")
			file->title = tagvalue;
		else if (tagname == "Artist")
			file->artist = tagvalue;
		else if (tagname == "Album")
			file->album = tagvalue;
		else if (tagname != "File" && mOrdProps.contains(tagname))
			file->tags[tagname] = tagvalue;
	}

	qAddFile.bindValue(":filename", filename);
	qAddFile.bindValue(":title", file->title);
	qAddFile.bindValue(":artist", file->artist);
	qAddFile.bindValue(":album", file->album);
	qAddFile.exec();

	// TODO check if it works
	file->mId = qAddFile.lastInsertId().toInt();

	mFiles[file->id()] = file;
	mFileIds[file->filename] = file->id();

	return file;
}
