#include "smsservice.h"
#include <klocale.h>

#include <kmessagebox.h>
#include <qlabel.h>
#include <qwidget.h>

bool SMSService::send(QString nr, QString message)
{
	KMessageBox::sorry(0L, QString("Should send:\n%1\nto: %2").arg(message).arg(nr), "Not implemented yet");
	return false;
}

QWidget* SMSService::configureWidget()
{
	return new QLabel("Testing...", 0);
}
