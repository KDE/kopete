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
class KopeteHistoryDialog;
class KopeteMessageManager;
class KopeteMetaContact;

class KActionCollection;
class KAction;


class SMSContact : public KopeteContact
{
	Q_OBJECT
public:
	SMSContact( SMSProtocol* protocol, const QString &phoneNumber,
		const QString &displayName, KopeteMetaContact *parent );
	~SMSContact();

	virtual QString id() const;

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

	QString servicePrefsString();
	void setServicePrefsString(QString servicePrefs);

public slots:

	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void execute();
	virtual void slotViewHistory();
	void slotCloseHistoryDialog();
	void slotSendMessage(const KopeteMessage &msg);

private slots:
	void userPrefs();
	void messageSent(const KopeteMessage&);

private:
	KopeteMessageManager* msgManager();
	void initActions();

	KActionCollection* m_actionCollection;
	KAction* m_actionPrefs;

	QString m_phoneNumber;
	QString m_serviceName;
	QMap<QString, QString> m_servicePrefs;
	SMSProtocol* m_protocol;

	KopeteHistoryDialog* m_historyDialog;

	KopeteMessageManager* m_msgManager;
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

