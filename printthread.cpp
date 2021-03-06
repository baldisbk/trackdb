#include "printthread.h"

#include <QtPrintSupport/QPrintDialog>

PrintThread::PrintThread(const QStringList &texts, QPrinter *printer):
	mTexts(texts), mPrinter(printer)
{}

void PrintThread::run()
{
	foreach(QString text, mTexts) {
		QTextDocument doc;
		QFont fnt("Courier", 10);
		doc.setDefaultFont(fnt);
		doc.setHtml(text);
		doc.print(mPrinter);
	}
	delete mPrinter;
}


PrintHelper::PrintHelper(QObject *parent): QObject(parent)
{
	mPrintThread = NULL;
}

PrintHelper::~PrintHelper()
{
	if (mPrintThread) {
		mPrintThread->terminate();
		mPrintThread->wait();
		mPrintThread->deleteLater();
	}
}

bool PrintHelper::print(QStringList texts)
{
	QPrinter *printer = new QPrinter;
	printer->setOrientation(QPrinter::Portrait);
	QPrintDialog dialog(printer);
	if (dialog.exec() == QDialog::Accepted) {
		if (mPrintThread == NULL) {
			mPrintThread = new PrintThread(texts, printer);
			connect(
				mPrintThread, SIGNAL(finished()),
				this, SLOT(onPrintFinished()));
			mPrintThread->start();
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void PrintHelper::onPrintFinished()
{
	delete mPrintThread;
	mPrintThread = NULL;
	emit printFinished();
}
