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
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

class KActionCollection;
class KAction;

class SMSContact : public Kopete::Contact
{
	Q_OBJECT
public:
	SMSContact( Kopete::Account* _account, const QString &phoneNumber,
		const QString &displayName, Kopete::MetaContact *parent );

	QPtrList<KAction>* customContextMenuActions();

	const QString &phoneNumber();
	void setPhoneNumber( const QString phoneNumber );
	const QString qualifiedNumber();

	/**
	 * Serialize contact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData,
		QMap<QString, QString> &addressBookData );

	Kopete::ChatSession* manager( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CanCreate );

public slots:
	virtual void slotUserInfo();
	virtual void deleteContact();
	void slotSendingSuccess(const Kopete::Message &msg);
	void slotSendingFailure(const Kopete::Message &msg, const QString &error);

private slots:
	void userPrefs();
	void slotChatSessionDestroyed();

private:
	KAction* m_actionPrefs;

	QString m_phoneNumber;

	Kopete::ChatSession* m_msgManager;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

