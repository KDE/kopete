/*
  smsclient.h  -  SMS Plugin

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
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

#include <QStringList>


class SMSClientPrefsUI;
class QGridLayout;
class K3Process;

class SMSClient : public SMSService
{
	Q_OBJECT
public:
	SMSClient(Kopete::Account* account);
	~SMSClient();

	void send(const Kopete::Message& msg);
	void setWidgetContainer(QWidget* parent, QGridLayout* container);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void slotReceivedOutput(K3Process*, char  *buffer, int  buflen);
	void slotSendFinished(K3Process* p);
signals:
	void messageSent(const Kopete::Message &);

private:
	QWidget* configureWidget(QWidget* parent);

	SMSClientPrefsUI* prefWidget;
	QStringList providers();
	QStringList output;

	Kopete::Message m_msg;

	QString m_description;
} ;

#endif //SMSCLIENT_H
