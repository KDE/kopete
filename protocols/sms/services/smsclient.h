/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSCLIENT_H
#define SMSCLIENT_H

#include "smsservice.h"
#include "kopetemessage.h"

#include <qobject.h>
#include <qstringlist.h>

class SMSClientPrefsUI;
class SMSContact;
class QListViewItem;
class KProcess;

class SMSClient : public SMSService
{
	Q_OBJECT
public:
	SMSClient(KopeteAccount* account);
	~SMSClient();

	void send(const KopeteMessage& msg);
	void setWidgetContainer(QWidget* parent, QGridLayout* container);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void slotReceivedOutput(KProcess*, char  *buffer, int  buflen);
	void slotSendFinished(KProcess* p);
signals:
	void messageSent(const KopeteMessage &);

private:
	QWidget* configureWidget(QWidget* parent);

	SMSClientPrefsUI* prefWidget;
	QStringList providers();
	QStringList output;

	KopeteMessage m_msg;

	QString m_description;
} ;

#endif //SMSCLIENT_H
