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

#ifndef SMSCONTACT_H
#define SMSCONTACT_H

#include "kopetecontact.h"
#include "kopetemessage.h"

#include <qstring.h>

class SMSAccount;
class KopeteMessageManager;
class KopeteMetaContact;

class KActionCollection;
class KAction;

class SMSContact : public KopeteContact
{
	Q_OBJECT
public:
	SMSContact( KopeteAccount* _account, const QString &phoneNumber,
		const QString &displayName, KopeteMetaContact *parent );

	QPtrList<KAction>* customContextMenuActions();

	const QString &phoneNumber();
	void setPhoneNumber( const QString phoneNumber );
	const QString qualifiedNumber();

	/**
	 * Serialize contact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData,
		QMap<QString, QString> &addressBookData );

	virtual bool isReachable();

	KopeteMessageManager* manager( bool canCreate = false );

public slots:
	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	void slotSendMessage(KopeteMessage &msg);

private slots:
	void slotSendingSuccess(const KopeteMessage &msg);
	void slotSendingFailure(const KopeteMessage &msg, const QString &error);

signals:
	void messageSuccess();

private slots:
	void userPrefs();
	void slotMessageManagerDestroyed();

private:
	KAction* m_actionPrefs;

	QString m_phoneNumber;

	KopeteMessageManager* m_msgManager;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

