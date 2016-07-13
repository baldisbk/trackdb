#include "printthread.h"

#include <QtPrintSupport/QPrintDialog>

PrintThread::PrintThread(const QString &text, QPrinter *printer):
	mText(text), mPrinter(printer)
{}

void PrintThread::run()
{
	QTextDocument doc;
	QFont fnt("Courier", 10);
	doc.setDefaultFont(fnt);
	doc.setHtml(mText);
	doc.print(mPrinter);
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

bool PrintHelper::print(QString text)
{
	QPrinter *printer = new QPrinter;
	printer->setOrientation(QPrinter::Portrait);
	QPrintDialog dialog(printer);
	if (dialog.exec() == QDialog::Accepted) {
		if (mPrintThread == NULL) {
			mPrintThread = new PrintThread(text, printer);
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
