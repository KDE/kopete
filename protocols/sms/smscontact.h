/*
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSCONTACT_H
#define SMSCONTACT_H

#include "kopetecontact.h"
#include "kopetemessage.h"

#include <qstring.h>

class SMSProtocol;
class KopeteMessageManager;
class KopeteMetaContact;

class KActionCollection;
class KAction;


class SMSContact : public KopeteContact
{
	Q_OBJECT
public:
	SMSContact( SMSProtocol* _protocol, const QString &phoneNumber,
		const QString &displayName, KopeteMetaContact *parent );
	~SMSContact();

	KActionCollection* customContextMenuActions();

	ContactStatus status() const;
	int importance() const;

	virtual bool isReachable() { return true; };

	QString phoneNumber();
	void setPhoneNumber( const QString phoneNumber );

	QString serviceName();
	void setServiceName(QString name);

	QString servicePref(QString name);
	void setServicePref(QString name, QString value);
	void deleteServicePref(QString name);
	void clearServicePrefs();

	QStringList servicePrefs();
	void setServicePrefs(QStringList prefs);

	/**
	 * Serialize contact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData,
		QMap<QString, QString> &addressBookData );

	KopeteMessageManager* manager();
public slots:

	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	void slotSendMessage(KopeteMessage &msg);
signals:
	void messageSuccess();

private slots:
	void userPrefs();
	void messageSent(KopeteMessage&);
	void slotMessageManagerDestroyed();

private:
	void initActions();

	KActionCollection* m_actionCollection;
	KAction* m_actionPrefs;

	QString m_phoneNumber;
	QString m_serviceName;
	QMap<QString, QString> m_servicePrefs;

	KopeteMessageManager* m_msgManager;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

