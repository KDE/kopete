/*
  AIMAccount - Oscar Protocol Account

  Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>

  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************

*/

#ifndef AIMACCOUNT_H
#define AIMACCOUNT_H

#include <qdict.h>
#include <qstring.h>
#include <qwidget.h>
#include "oscartypeclasses.h"

#include "oscaraccount.h"
#include "oscarmyselfcontact.h"


namespace Kopete
{
class Contact;
class Group;
}

class KAction;
class OscarContact;
class AIMContact;
class AIMAccount;
class AIMJoinChatUI;

class AIMMyselfContact : public OscarMyselfContact
{
public:
	AIMMyselfContact( AIMAccount *acct );
	void userInfoUpdated();
	void setOwnProfile( const QString& newProfile );
	QString userProfile();
	void setLastAwayMessage( const QString& msg) {m_lastAwayMessage = msg;}
	QString lastAwayMessage() { return m_lastAwayMessage; };
	
private:
	QString m_profileString;
	AIMAccount* m_acct;
	/**
	 * There has GOT to be a better way to get this away message
	 */
	QString m_lastAwayMessage;

};

class AIMAccount : public OscarAccount
{
Q_OBJECT
	
public:
	AIMAccount(Kopete::Protocol *parent, QString accountID, const char *name=0L);
	virtual ~AIMAccount();
	
	// Accessor method for the action menu
	virtual KActionMenu* actionMenu();
	
	/** Reimplementation from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus&, const QString& ) {}
	
	void setAway(bool away, const QString &awayReason);
	
	virtual void connectWithPassword( const QString &password );
	
	void setUserProfile(const QString &profile);
	
public slots:
	void slotEditInfo();
	void slotGoOnline();
	
	void globalIdentityChanged( const QString&, const QVariant& );
	void sendBuddyIcon();
    void slotJoinChat();

	
protected slots:
	void slotGoAway(const QString&);
    void joinChatDialogClosed();
	
	virtual void disconnected( Kopete::Account::DisconnectReason reason );
	
	virtual void messageReceived( const Oscar::Message& message );
	
protected:
	
	/**
	* Implement virtual method from OscarAccount
	* This allows OscarAccount to take care of adding new contacts
	*/
	OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem );

	QString sanitizedMessage( const Oscar::Message& message );
	
private:
    AIMJoinChatUI* m_joinChatDialog;
};
#endif
//kate: tab-width 4; indent-mode csands;
