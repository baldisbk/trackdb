#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QDateTime>

enum FillState {
	fullState = 0,	// IFM
	noInfoState,	// _FM
	noFileState,	// I__
	noMinusState,	// IF_
	oneFileState,	// _F_
	emptyState	// ___
};			// __M and I_M are impossible

class TracksModel;

struct File {
	File(): mId(-1), mTrack(-1) {}
	QString filename;
	QString title;
	QString artist;
	QString album;
	QMap<QString, QString> tags;
	int id() const {return mId;}
	int track() const {return mTrack;}
	QDateTime lastPlayed;

private:
	int mId;
	int mTrack;

	friend class TracksModel;
};

struct Property {
	enum Type {
		Ordinary,
		Big,
		Category
	};

	Property(): type(Ordinary), mId(-1) {}
	int id() const {return mId;}
	Type type;
	QString name;

private:
	int mId;
	friend class TracksModel;
};

struct Tag {
	Tag(): mId(-1) {}
	int id() const {return mId;}
	QString type;
	QString name;

private:
	int mId;
	friend class TracksModel;
};

struct Record {
	Record();
	Record(File* file, int id);

	int id() const;
	QString main() const;
	QString minus() const;
	QStringList propList() const;
	QVariant property(QString name) const;
	bool setProperty(QString name, QVariant value);

	QStringList tags() const;
	QStringList files() const;

	QString title;
	QString artist;
	QString album;
	QDateTime lastChanged;

	FillState fill() const;

	bool match(File* file) const;
	bool addFile(File *file);
	bool removeFile(File *file);
	void addInfo();

private:
	FillState mFill;
	QStringList mTags;
	QMap<QString, QString> mProperties;
	QMap<QString, QStringList> mCategories;
	QStringList mFiles;

	int mId;
	QString mMain;
	QString mMinus;

	bool mChanged;

	friend class TracksModel;
};

#endif // DATABASE_H
