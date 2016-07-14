#include "tracksmodel.h"

#include <QColor>
#include <QDir>
#include <QIcon>
#include <QMessageBox>

#include <QSqlQuery>
#include <QSqlRecord>

#include <QProcess>
#include <QCoreApplication>

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
		mSelectedTrack->mMinus = mSelectedTrack->prime();
	mSelectedTrack->mMain = file;
	updateRecord();
	return true;
}

bool TracksModel::setMinusFile(QString file)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->files().contains(file))
		if (!addFile(file))
			return false;
	if (mSelectedTrack->prime() == file)
		mSelectedTrack->mMain = mSelectedTrack->minus();
	mSelectedTrack->mMinus = file;
	updateRecord();
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
		mSelectedTrack->addFile(f);
	}
	updateRecord();
	emit recordSelected(mSelectedTrack);
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
	updateRecord();
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

		QSqlDatabase db = QSqlDatabase::database();

		QSqlQuery qAddTag(db);
		qAddTag.prepare(
			"INSERT INTO tag (tag, property) VALUES (:tag, 0)");
		qAddTag.bindValue(":tag", tag);
		qAddTag.exec();

		t->mId = qAddTag.lastInsertId().toInt();

		mTags[tag] = t;
		mCatTags[QString()].append(tag);
		emit tagsChanged();
	}
	if (mSelectedTrack->tags().contains(tag))
		return false;
	mSelectedTrack->mTags.append(tag);
	updateRecord();
	return true;
}

bool TracksModel::removeTag(QString tag)
{
	if (!mSelectedTrack)
		return false;
	if (!mSelectedTrack->tags().contains(tag))
		return false;
	mSelectedTrack->mTags.removeAll(tag);
	updateRecord();
	return true;
}

QString TracksModel::printHtml(QString property) const
{
	if (!mSelectedTrack)
		return QString();
	QString res;
	res += "<html><body>";
	if (!property.isEmpty()) {
		res += mSelectedTrack->property(property).toString();
		res += "</body></html>";
		return res;
	}
	res += "<table cellspacing=0 cellpadding=2 border=1>";
	res += "<tr>";
	res += "<td>";
	res += "Header";
	res += "</td>";
	res += "</tr>";
	foreach (Property* prop, mProperties) {
		if (prop->type == Property::Big) continue;
		QVariant val = mSelectedTrack->property(prop->name);
		if (val.isNull()) continue;
		QString strval;
		if (val.type() == QVariant::String)
			strval = val.toString();
		if (val.type() == QVariant::StringList)
			strval = val.toStringList().join(", ");
		res += "<tr><td><b>";
		res += prop->name;
		res += "</b></td><td>";
		res += strval;
		res += "</td></tr>";
	}
	res += "</table></font></body></html>";
	return res;
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
	return mCatProps + mOrdProps;
}

QStringList TracksModel::allBigProperties() const
{
	return mBigProps;
}

QStringList TracksModel::allFiles() const
{
	return mFileIds.keys();
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
		if (rec->changed())
			return QColor(Qt::red);
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
	QStringList filters = QString(SUPPORTED_TYPES).split(" ");
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
		if (file->track() <= 0)
			fillFile(file);

		Record* rec = mTracks.value(file->track(), NULL);
		if (!rec) {
			bool found = false;
			foreach(Record* rec, mTracks) {
				if (rec->id() >= 0) continue;
				if (rec->match(file) && rec->addFile(file)) {
					found = true;
					file->mTrack = rec->id();
					rec->setChanged();
					break;
				}
			}
			if (!found) {
				rec = new Record(file, -mTracks.size() - 1);
				mTracks[rec->id()] = rec;
				mTrackIds.append(rec->id());
				file->mTrack = rec->id();
				rec->setChanged();
			}
		} else {
			bool nomain = rec->prime().isEmpty();
			bool nominus = rec->minus().isEmpty();
			rec->addFile(file);
			if (file->id() == qSelectFiles.value(7).toInt())
				rec->mMain = file->filename;
			else if (nomain)
				rec->mMain.clear();
			if (file->id() == qSelectFiles.value(8).toInt())
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
		"SELECT track.id, name, value as val "
		"FROM track "
		"JOIN trackproperty ON track.id=trackproperty.track "
		"JOIN property ON trackproperty.property=property.id ");
	while(qSelectPropForTracks.next()) {
		Record *rec = mTracks.value(qSelectPropForTracks.value(0).toInt(), NULL);
		if (!rec)
			continue;
		rec->mProperties[qSelectPropForTracks.value(1).toString()] =
			qSelectPropForTracks.value(2).toString();
	}

	QSqlQuery qSelectCatsForTracks(db);
	// zats wan grit kwery!!! B-)
	qSelectCatsForTracks.exec(
		"SELECT track.id, name, tag.tag as val "
		"FROM track "
		"JOIN tracktag ON tracktag.track=track.id "
		"JOIN tag ON tracktag.tag=tag.id "
		"JOIN property ON tag.property=property.id"
		);
	while(qSelectCatsForTracks.next()) {
		Record *rec = mTracks.value(qSelectCatsForTracks.value(0).toInt(), NULL);
		if (!rec)
			continue;
		rec->mCategories[qSelectCatsForTracks.value(1).toString()].append(
			qSelectCatsForTracks.value(2).toString());
	}

	endResetModel();
	emit recordSelected(mSelectedTrack);
}

void TracksModel::clearFiles()
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery qDeleteFiles(db);
	qDeleteFiles.exec("DELETE FROM file");
	qDeleteFiles.exec("UPDATE track SET main=0, minus=0");

	beginResetModel();
	mFileIds.clear();
	mFiles.clear();
	QList<int> toDelete;
	foreach(Record* track, mTracks) {
		track->mMain.clear();
		track->mMinus.clear();
		track->mFiles.clear();
		track->mFill = noFileState;
		if (track->id() < 0)
			toDelete.append(track->id());
	}
	foreach(int id, toDelete) {
		mTrackIds.removeAll(id);
		mTracks.remove(id);
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

		QCoreApplication::processEvents();

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
			if (rec->match(newFile) && rec->addFile(newFile)) {
				found = true;
				newFile->mTrack = rec->id();
				rec->setChanged();
				break;
			}
		}
		if (!found) {
			Record* rec = new Record(newFile, -mTracks.size() - 1);
			mTracks[rec->id()] = rec;
			mTrackIds.append(rec->id());
			rec->setChanged();
			newFile->mTrack = rec->id();
		}

		emit progress(number++);
	}
	emit progressEnd();
	endResetModel();
}

void TracksModel::importDB(const QString &path)
{
	// TODO
}

bool TracksModel::checkChanges()
{
	if (mSelectedTrack && mSelectedTrack->changed()) {
		QMessageBox::StandardButton btn =
			QMessageBox::question(
				dynamic_cast<QWidget*>(QObject::parent()),
				tr("Who are you? What do you want?"),
				tr("You have made changes to selected record. "
					"Do you want to save changes?"),
				QMessageBox::Discard | QMessageBox::Save
					/*| QMessageBox::Cancel*/);
		switch (btn) {
		case QMessageBox::Discard:
			revertRecord();
			break;
		case QMessageBox::Save:
			saveRecord();
			break;
		case QMessageBox::Cancel: {
			// save to origin, dont save to DB
			// TODO it l8r
		}
		default:;
		}
	}
	return true;
}

void TracksModel::selectRecord(const QModelIndex &index)
{
	if (!checkChanges())
		; // TODO return selection
	Record* origin = recordForIndex(index);
	Record* newrec = NULL;
	if (origin) {
		newrec = new Record;
		*newrec = *origin;
	}
	emit recordSelected(newrec);
	if (mSelectedTrack)
		delete mSelectedTrack;
	mSelectedTrack = newrec;
}

int TracksModel::insertTrack()
{
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery qAddTrack(db);
	qAddTrack.prepare(
		"INSERT INTO track (title, artist, album, main, minus, lastchange) "
		"VALUES (:title, :artist, :album, :main, :minus, :lc)");

	qAddTrack.bindValue(":title", mSelectedTrack->title);
	qAddTrack.bindValue(":artist", mSelectedTrack->artist);
	qAddTrack.bindValue(":album", mSelectedTrack->album);
	if (mSelectedTrack->prime().isEmpty())
		qAddTrack.bindValue(":main", QVariant());
	else
		qAddTrack.bindValue(":main", mFileIds.value(mSelectedTrack->prime(), 0));
	if (mSelectedTrack->prime().isEmpty())
		qAddTrack.bindValue(":minus", QVariant());
	else
		qAddTrack.bindValue(":minus", mFileIds.value(mSelectedTrack->minus(), 0));
	qAddTrack.bindValue(":lc", mSelectedTrack->lastChanged.toString());
	qAddTrack.exec();

	int id = qAddTrack.lastInsertId().toInt();

	QSqlQuery qAddTag(db);
	qAddTag.prepare("INSERT INTO tracktag (track, tag) VALUES (:track, :tag)");
	foreach(QString tag, mSelectedTrack->mTags) {
		qAddTag.bindValue(":track", id);
		qAddTag.bindValue(":tag", mTags.value(tag)->id());
		qAddTag.exec();
	}
	foreach(QStringList cats, mSelectedTrack->mCategories) {
		foreach(QString cat, cats) {
			qAddTag.bindValue(":track", id);
			qAddTag.bindValue(":tag", mTags.value(cat)->id());
			qAddTag.exec();
		}
	}

	QSqlQuery qAddProp(db);
	qAddProp.prepare(
		"INSERT INTO trackproperty (track, property, value) "
		"VALUES (:track, :prop, :value)");
	foreach(QString prop, mSelectedTrack->mProperties.keys()) {
		qAddProp.bindValue(":track", id);
		qAddProp.bindValue(":prop", mProperties.value(prop)->id());
		qAddProp.bindValue(":value", mSelectedTrack->mProperties[prop]);
		qAddProp.exec();
	}

	QSqlQuery qSetFile(db);
	qSetFile.prepare("UPDATE file SET track=:track WHERE id=:id");
	foreach(QString file, mSelectedTrack->files()) {
		int fid = mFileIds[file];
		qSetFile.bindValue(":track", id);
		qSetFile.bindValue(":id", fid);
		qSetFile.exec();
	}

	return id;
}

void TracksModel::updateTrack(Record *rec, Record* origin)
{
	if (rec->id() < 0)
		return;
	QSqlDatabase db = QSqlDatabase::database();

	RecordChanges* rc = rec->compare(origin);

	if (rc->info) {
		QSqlQuery qUpdTrack(db);
		qUpdTrack.prepare(
			"UPDATE track SET "
			"title=:title, artist=:artist, album=:album, "
			"main=:main, minus=:minus, lastchange=:lc "
			"WHERE id=:id");

		qUpdTrack.bindValue(":id", rec->id());
		qUpdTrack.bindValue(":title", rec->title);
		qUpdTrack.bindValue(":artist", rec->artist);
		qUpdTrack.bindValue(":album", rec->album);
		if (rec->prime().isEmpty())
			qUpdTrack.bindValue(":main", QVariant());
		else
			qUpdTrack.bindValue(":main", mFileIds.value(rec->prime(), 0));
		if (rec->prime().isEmpty())
			qUpdTrack.bindValue(":minus", QVariant());
		else
			qUpdTrack.bindValue(":minus", mFileIds.value(rec->minus(), 0));
		qUpdTrack.bindValue(":lc", rec->lastChanged.toString());
		qUpdTrack.exec();
	}

	QSqlQuery qAddTag(db);
	qAddTag.prepare("INSERT INTO tracktag (track, tag) VALUES (:track, :tag)");
	foreach(QString tag, rc->addedTags) {
		qAddTag.bindValue(":track", rec->id());
		qAddTag.bindValue(":tag", mTags[tag]->id());
		qAddTag.exec();
	}

	QSqlQuery qRemoveTag(db);
	qRemoveTag.prepare("DELETE FROM tracktag WHERE track=:track AND tag=:tag");
	foreach(QString tag, rc->removedTags) {
		qRemoveTag.bindValue(":track", rec->id());
		qRemoveTag.bindValue(":tag", mTags[tag]->id());
		qRemoveTag.exec();
	}

	QSqlQuery qSetProperty(db);
	qSetProperty.prepare(
		"UPDATE trackproperty SET value=:value "
		"WHERE track=:track AND property=:prop");
	foreach(QString prop, rc->changedProps) {
		if (!mProperties.contains(prop)) continue;
		qSetProperty.bindValue(":value", rec->property(prop).toString());
		qSetProperty.bindValue(":track", rec->id());
		qSetProperty.bindValue(":prop", mProperties[prop]->id());
		qSetProperty.exec();
	}

	QSqlQuery qRemoveProp(db);
	qRemoveProp.prepare(
		"DELETE FROM trackproperty "
		"WHERE track=:track AND property=:prop");
	foreach(QString prop, rc->removedProps) {
		if (!mProperties.contains(prop)) continue;
		qRemoveProp.bindValue(":track", rec->id());
		qRemoveProp.bindValue(":prop", mProperties[prop]->id());
		qRemoveProp.exec();
	}

	QSqlQuery qAddProp(db);
	qAddProp.prepare(
		"INSERT INTO trackproperty (track, property, value) "
		"VALUES (:track, :prop, :value)");
	foreach(QString prop, rc->addedProps) {
		if (!mProperties.contains(prop)) continue;
		qAddProp.bindValue(":track", rec->id());
		qAddProp.bindValue(":value", rec->property(prop));
		qAddProp.bindValue(":prop", mProperties[prop]->id());
		qAddProp.exec();
	}

	QSqlQuery qSetFile(db);
	qSetFile.prepare("UPDATE file SET track=:track WHERE id=:id");
	foreach(QString file, rc->removedFiles) {
		int id = mFileIds[file];
		// they will be re-bound later
		qSetFile.bindValue(":track", 0);
		qSetFile.bindValue(":id", id);
		qSetFile.exec();
	}
	foreach(QString file, rc->removedFiles) {
		int id = mFileIds[file];
		qSetFile.bindValue(":track", rec->id());
		qSetFile.bindValue(":id", id);
		qSetFile.exec();
	}
}

void TracksModel::saveRecord()
{
	if (!mSelectedTrack)
		return;
	Record* origin = mTracks.value(mSelectedTrack->id(), NULL);
	mSelectedTrack->lastChanged = QDateTime::currentDateTime();
	if (!origin) {
		// this is a new record
		int id = insertTrack();
		beginInsertRows(QModelIndex(), mTrackIds.size(), mTrackIds.size());
		Record* newrec = new Record;
		mTracks[id] = newrec;
		if (newrec->prime() == 0)
			newrec->mFill = noFileState;
		else if (newrec->minus() == 0)
			newrec->mFill = noMinusState;
		else
			newrec->mFill = fullState;
		mSelectedTrack->mId = id;
		mSelectedTrack->mChanged = false;
		*newrec = *mSelectedTrack;
		mTrackIds.append(id);
		endInsertRows();
	} else {
		if (origin->id() < 0) {
			// this is a new record with existing file
			int id = insertTrack();
			mTracks.remove(origin->id());
			Record* newrec = new Record;
			mTracks[id] = newrec;
			mSelectedTrack->mId = id;
			mSelectedTrack->mChanged = false;
			*newrec = *mSelectedTrack;
			if (newrec->prime() == 0)
				newrec->mFill = noFileState;
			else if (newrec->minus() == 0)
				newrec->mFill = noMinusState;
			else
				newrec->mFill = fullState;
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
			// this is an existing record
			updateTrack(mSelectedTrack, origin);
			int ind = mTrackIds.indexOf(origin->id());
			mSelectedTrack->mChanged = false;
			*origin = *mSelectedTrack;
			emit dataChanged(index(ind, 0), index(ind, columnCount()-1));
		}
	}

	// re-bind files
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery qUpdFile(db);
	qUpdFile.prepare("UPDATE file SET track=:track WHERE id=:id");

	QStringList files = mSelectedTrack->files();
	foreach(QString file, files) {
		File* f = mFiles.value(mFileIds.value(file, 0), NULL);
		if (!f) continue;
		if (f->track() != mSelectedTrack->id()) {
			Record* old = mTracks.value(f->track(), NULL);
			if (old) {
				if (old->id() < 0 && old->files().size() == 1) {
					// remove track
					int ind = mTrackIds.indexOf(old->id());
					beginRemoveRows(QModelIndex(), ind, ind);
					mTrackIds.removeAt(ind);
					endRemoveRows();
					mTracks.remove(old->id());
					delete old;
				} else {
					// update track
					Record* tmp = new Record;
					*tmp = *old;
					old->removeFile(f);
					updateTrack(old, tmp);
					delete tmp;
				}
			}
			f->mTrack = mSelectedTrack->id();
			qUpdFile.bindValue(":track", mSelectedTrack->id());
			qUpdFile.bindValue(":id", f->id());
			qUpdFile.exec();
		}
	}
	emit recordSelected(mSelectedTrack);
	emit recordChanged();
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
		mSelectedTrack->mChanged = false;
		origin->mChanged = false;
	}
	emit recordSelected(mSelectedTrack);
}

void TracksModel::newRecord()
{
	if (!checkChanges())
		return;
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
		QSqlDatabase db = QSqlDatabase::database();
		QSqlQuery qUpdFile(db);
		qUpdFile.prepare("UPDATE files SET lastplayed=:lp WHERE id=:id");
		qUpdFile.bindValue(":lp", f->lastPlayed.toString());
		qUpdFile.bindValue(":id", f->id());
		qUpdFile.exec();
	}
}

void TracksModel::updateRecord()
{
	if (mSelectedTrack)
		mSelectedTrack->setChanged();
	emit recordChanged();
}

void TracksModel::fillFile(File* file)
{
	QSqlDatabase db = QSqlDatabase::database();

	QSqlQuery qAddProp(db);
	qAddProp.prepare(
		"INSERT INTO property (name, big, fixed) "
		"VALUES (:name, 0, 0)");

	QProcess prc;
	prc.start("./id3", QStringList() << file->filename);
	if (!prc.waitForStarted() || !prc.waitForFinished()) {
		return;
	}

	QByteArray res = prc.readAll();
	QStringList lines = QString::fromLocal8Bit(res).split('\n');

	foreach(QString line, lines) {
		int split = line.indexOf(":");
		if (split < 0)
			continue;
		QString tagname = line.left(split);
		QString tagvalue = line.remove(0, split+1).simplified();
		if (tagname.isEmpty() || tagvalue.isEmpty())
			continue;
		if (tagname == "Title")
			file->title = tagvalue;
		else if (tagname == "Artist")
			file->artist = tagvalue;
		else if (tagname == "Album")
			file->album = tagvalue;
		else if (tagname != "File" && tagname != "Metadata") {
			if (!mOrdProps.contains(tagname)) {
				if (!mProperties.contains(tagname)) {
					qAddProp.bindValue(":name", tagname);
					qAddProp.exec();
					Property* prop = new Property;
					prop->mId = qAddProp.lastInsertId().toInt();
					prop->type = Property::Ordinary;
					prop->name = tagname;
					mProperties[tagname] = prop;
					mOrdProps.append(tagname);
					emit dbChanged();
				}
			}
			if (mOrdProps.contains(tagname))
				file->tags[tagname] = tagvalue;
		}
	}
}

File *TracksModel::readFile(QString filename)
{
	File* file = NULL;
	bool hasFile = false;
	if (mFileIds.contains(filename)) {
		file = mFiles[mFileIds[filename]];
		hasFile = true;
		if (file->track() != 0)
			return file;
	}

	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery qAddFile(db);
	qAddFile.prepare(
		"INSERT INTO file (filename, title, artist, album) "
		"VALUES (:filename, :title, :artist, :album)");

	if (!file)
		file = new File;
	file->filename = filename;
	fillFile(file);

	if (!hasFile) {
		qAddFile.bindValue(":filename", filename);
		qAddFile.bindValue(":title", file->title);
		qAddFile.bindValue(":artist", file->artist);
		qAddFile.bindValue(":album", file->album);
		qAddFile.exec();

		file->mId = qAddFile.lastInsertId().toInt();

		mFiles[file->id()] = file;
		mFileIds[file->filename] = file->id();
	}

	return file;
}
