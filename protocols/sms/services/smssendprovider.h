/*
  smssendprovider.h  -  SMS Plugin

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

#ifndef SMSSENDPROVIDER_H
#define SMSSENDPROVIDER_H


#include <QStringList>
#include <QList>
#include <QByteArray>

#include <klineedit.h>

#include "kopetemessage.h"

#include "smsaccount.h"

class K3Process;
namespace Kopete { class Account; }

class SMSSendProvider : public QObject
{
	Q_OBJECT
public:
	SMSSendProvider(const QString& providerName, const QString& prefixValue, Kopete::Account* account, QObject* parent = 0);
	~SMSSendProvider();

	void setAccount(Kopete::Account *account);

	int count();
	QString name(int i);
	const QString& value(int i);
	const QString& description(int i);
	bool isHidden(int i) const;

	void save(const QList<KLineEdit*>& args);
	void send(const Kopete::Message& msg);

	int maxSize();
private slots:
	void slotReceivedOutput(K3Process*, char  *buffer, int  buflen);
	void slotSendFinished(K3Process*);
private:
	QStringList names;
	QStringList descriptions;
	QStringList values;
	QList<bool> isHiddens;

	int messagePos;
	int telPos;
	int m_maxSize;

	QString provider;
	QString prefix;
	QByteArray output;

	Kopete::Account* m_account;

	Kopete::Message m_msg;

	bool canSend;
signals:
	void messageSent(const Kopete::Message& msg);
	void messageNotSent(const Kopete::Message& msg, const QString &error);
} ;

#endif //SMSSENDPROVIDER_H
