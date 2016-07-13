#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QTextDocument>
#include <QtPrintSupport/QPrinter>
#include <QThread>

class PrintThread : public QThread {
public:
	PrintThread(const QString& text, QPrinter* printer);
	void run();
private:
	QString mText;
	QPrinter *mPrinter;
};

class PrintHelper : public QObject {
	Q_OBJECT
public:
	PrintHelper(QObject* parent = NULL);
	~PrintHelper();

	bool print(QString text);

signals:
	void printFinished();

private slots:
	void onPrintFinished();

private:
	PrintThread* mPrintThread;

};

#endif
