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

namespace AIM
{
	namespace PrivacySettings
	{
		enum { AllowAll = 0, AllowMyContacts, AllowPremitList, BlockAll, BlockAIM, BlockDenyList };
	}
}

namespace Kopete
{
class Contact;
class Group;
class ChatSession;
}

class KAction;
class OscarContact;
class AIMContact;
class AIMAccount;
class AIMJoinChatUI;
class AIMChatSession;
class OscarVisibilityDialog;

class AIMMyselfContact : public OscarMyselfContact
{
Q_OBJECT
public:
	AIMMyselfContact( AIMAccount *acct );
	void userInfoUpdated();
	void setOwnProfile( const QString& newProfile );
	QString userProfile();
	void setLastAwayMessage( const QString& msg) {m_lastAwayMessage = msg;}
	QString lastAwayMessage() { return m_lastAwayMessage; };

    virtual Kopete::ChatSession* manager( Kopete::Contact::CanCreateFlags = Kopete::Contact::CannotCreate,
                                          WORD exchange = 0, const QString& room = QString::null);

public slots:
    void sendMessage( Kopete::Message&, Kopete::ChatSession* session );
    void chatSessionDestroyed( Kopete::ChatSession* );

private:
	QString m_profileString;
	AIMAccount* m_acct;
	/**
	 * There has GOT to be a better way to get this away message
	 */
	QString m_lastAwayMessage;
    QValueList<Kopete::ChatSession*> m_chatRoomSessions;


};

class AIMAccount : public OscarAccount
{
Q_OBJECT

public:
	AIMAccount(Kopete::Protocol *parent, QString accountID, const char *name=0L);
	virtual ~AIMAccount();

	// Accessor method for the action menu
	virtual KActionMenu* actionMenu();

	void setAway(bool away, const QString &awayReason = QString::null );

	virtual void connectWithPassword( const QString &password );

	void setUserProfile(const QString &profile);
	
	void setPrivacySettings( int privacy );

public slots:
	/** Reimplementation from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status, const QString& reason = QString::null );
	void slotEditInfo();
	void slotGoOnline();

	void slotGlobalIdentityChanged( const QString&, const QVariant& );
	void slotBuddyIconChanged();

	void slotJoinChat();

protected slots:
	void slotGoAway(const QString&);
    void joinChatDialogClosed( int );

	virtual void loginActions();
	virtual void disconnected( Kopete::Account::DisconnectReason reason );
	virtual void messageReceived( const Oscar::Message& message );

    void connectedToChatRoom( WORD exchange, const QString& roomName );
    void userJoinedChat( Oscar::WORD exchange, const QString& room, const QString& contact );
    void userLeftChat( Oscar::WORD exchange, const QString& room, const QString& contact );

	void slotSetVisiblility();
	void slotVisibilityDialogClosed();

protected:

	/**
	* Implement virtual method from OscarAccount
	* This allows OscarAccount to take care of adding new contacts
	*/
	OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem );

	QString sanitizedMessage( const QString& message );

private:
	// Set privacy tlv item
	void setPrivacyTLVs( BYTE privacy, DWORD userClasses );

    AIMJoinChatUI* m_joinChatDialog;
	OscarVisibilityDialog* m_visibilityDialog;
};
#endif
//kate: tab-width 4; indent-mode csands;
