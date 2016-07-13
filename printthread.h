#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QTextDocument>
#include <QtPrintSupport/QPrinter>
#include <QThread>

class PrintThread : public QThread {
public:
	PrintThread(const QStringList& texts, QPrinter* printer);
	void run();
private:
	QStringList mTexts;
	QPrinter *mPrinter;
};

class PrintHelper : public QObject {
	Q_OBJECT
public:
	PrintHelper(QObject* parent = NULL);
	~PrintHelper();

	bool print(QStringList texts);

signals:
	void printFinished();

private slots:
	void onPrintFinished();

private:
	PrintThread* mPrintThread;

};

#endif
