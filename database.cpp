#include "database.h"

#include <QDebug>

Record::Record(): mFill(noFileState), mId(0), mChanged(false) {}

Record::Record(File *file, int id)
{
	album = file->album;
	artist = file->artist;
	title = file->title;
	mFiles.append(file->filename);
	mFill = oneFileState;
	mMain = file->filename;
	mProperties = file->tags;
	mId = id;
	mChanged = false;
}

int Record::id() const {return mId;}

QString Record::main() const {return mMain;}

QString Record::minus() const {return mMinus;}

QStringList Record::propList() const {
	return mProperties.keys() + mCategories.keys();
}

QVariant Record::property(QString name) const
{
	if (mProperties.contains(name))
		return mProperties.value(name);
	if (mCategories.contains(name))
		return mCategories.value(name);
	return QVariant();
}

bool Record::setProperty(QString name, QVariant value)
{
	if (name.isEmpty()) {
		mTags = value.toStringList();
		return true;
	}
	if (value.type() == QVariant::String) {
		mProperties[name] = value.toString();
		return true;
	}
	if (value.type() == QVariant::StringList) {
		mCategories[name] = value.toStringList();
		return true;
	}
	return false;
}

QStringList Record::tags() const {return mTags;}

QStringList Record::files() const {return mFiles;}

FillState Record::fill() const {return mFill;}

bool Record::match(File *file) const
{
	return
		file->title == title &&
		file->album == album &&
			file->artist == artist;
}

bool Record::addFile(File* file)
{
	if (!file)
		return false;
	if (mFiles.contains(file->filename))
		return false;
	if (!match(file))
		return false;

	mFiles.append(file->filename);
	switch (mFill) {
	case noFileState:
		mFill = noMinusState;
		mMain = file->filename;
		break;
	case noMinusState:
		mFill = fullState;
		if (mMinus.isEmpty())
			mMinus = file->filename;
		break;
	case oneFileState:
		mFill = noInfoState;
		if (mMinus.isEmpty())
			mMinus = file->filename;
		break;
	default:;
	}
	return true;
}

bool Record::removeFile(File *file)
{
	if (!file)
		return false;
	if (!mFiles.contains(file->filename))
		return false;

	mFiles.removeAll(file->filename);
	if (mMain == file->filename)
		mMain.clear();
	if (mMinus == file->filename)
		mMinus.clear();
	switch (mFiles.size()) {
	case 0:
		if (mFill == oneFileState)
			mFill = emptyState;
		else
			mFill = noFileState;
		break;
	case 1:
		if (mFill == noInfoState)
			mFill = oneFileState;
		else
			mFill = noMinusState;
		mMinus.clear();
		mMain = mFiles.first();
		break;
	default:
		if (mMain.isEmpty())
			foreach(QString filename, mFiles)
				if (mMinus != filename) {
					mMain = filename;
					break;
				}
		if (mMinus.isEmpty())
			foreach(QString filename, mFiles)
				if (mMain != filename) {
					mMinus = filename;
					break;
				}
	}
	return true;
}

void Record::addInfo()
{
	switch (fill()) {
	case oneFileState: mFill = noMinusState;
	case noInfoState: mFill = fullState;
	default:;
	}
	mChanged = true;
}

RecordChanges *Record::compare(Record *other)
{
	RecordChanges* rc = new RecordChanges;

	rc->info =
		title != other->title ||
		album != other->album ||
		artist != other->artist ||
		lastChanged != other->lastChanged;

	foreach(QString tag, mTags)
		if (!other->mTags.contains(tag))
			rc->addedTags.append(tag);
	foreach(QString tag, other->mTags)
		if (!mTags.contains(tag))
			rc->removedTags.append(tag);

	QStringList thisCats, thatCats;
	foreach(QStringList cats, mCategories)
		thisCats.append(cats);
	foreach(QStringList cats, other->mCategories)
		thatCats.append(cats);

	foreach(QString cat, thisCats)
		if (!thatCats.contains(cat))
			rc->addedTags.append(cat);
	foreach(QString cat, thatCats)
		if (!thisCats.contains(cat))
			rc->removedTags.append(cat);

	foreach(QString file, mFiles)
		if (!other->mFiles.contains(file))
			rc->addedFiles.append(file);
	foreach(QString file, other->mFiles)
		if (!mFiles.contains(file))
			rc->removedFiles.append(file);

	foreach(QString prop, mProperties.keys())
		if (property(prop) != other->property(prop)) {
			if (other->property(prop).toString().isEmpty())
				rc->addedProps.append(prop);
			else
				rc->changedProps.append(prop);
		}
	foreach(QString prop, other->mProperties.keys())
		if (property(prop) != other->property(prop))
			if (property(prop).toString().isEmpty())
				rc->removedProps.append(prop);

	return rc;
}
