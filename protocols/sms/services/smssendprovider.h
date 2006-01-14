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

#ifndef SMSSENDPROVIDER_H
#define SMSSENDPROVIDER_H

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qlabel.h>
#include <qvaluelist.h>

#include <klineedit.h>

#include "kopetemessage.h"

#include "smsaccount.h"

class KProcess;
namespace Kopete { class Account; }
class SMSContact;

class SMSSendProvider : public QObject
{
	Q_OBJECT
public:
	SMSSendProvider(const QString& providerName, const QString& prefixValue, Kopete::Account* account, QObject* parent = 0, const char* name = 0);
	~SMSSendProvider();

	void setAccount(Kopete::Account *account);

	int count();
	const QString& name(int i);
	const QString& value(int i);
	const QString& description(int i);
	const bool isHidden(int i);

	void save(QPtrList<KLineEdit>& args);
	void send(const Kopete::Message& msg);

	int maxSize();
private slots:
	void slotReceivedOutput(KProcess*, char  *buffer, int  buflen);
	void slotSendFinished(KProcess*);
private:
	QStringList names;
	QStringList descriptions;
	QStringList values;
	QValueList<bool> isHiddens;

	int messagePos;
	int telPos;
	int m_maxSize;

	QString provider;
	QString prefix;
	QCString output;

	Kopete::Account* m_account;

	Kopete::Message m_msg;

	bool canSend;
signals:
	void messageSent(const Kopete::Message& msg);
	void messageNotSent(const Kopete::Message& msg, const QString &error);
} ;

#endif //SMSSENDPROVIDER_H
