/*
  AIMAccount - Oscar Protocol Account

  Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>

  Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qstring.h>
#include <qwidget.h>
#include "oscartypeclasses.h"

#include "oscaraccount.h"
#include "oscarmyselfcontact.h"
#include "oscarpresence.h"

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
class ChatSession;
class StatusMessage;
}

class KAction;
class KToggleAction;
class OscarContact;
class AIMAccount;
class AIMProtocol;
class AIMJoinChatUI;

class AIMMyselfContact : public OscarMyselfContact
{
Q_OBJECT
public:
	AIMMyselfContact( AIMAccount *acct );
	void userInfoUpdated();
	void setOwnProfile( const QString& newProfile );
	QString userProfile();

    virtual Kopete::ChatSession* manager( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CannotCreate );
    virtual Kopete::ChatSession* manager( Kopete::Contact::CanCreateFlags canCreate,
                                          Oscar::WORD exchange, const QString& room );

public slots:
    void sendMessage( Kopete::Message&, Kopete::ChatSession* session );
    void chatSessionDestroyed( Kopete::ChatSession* );

private:
	QString m_profileString;
	AIMAccount* m_acct;
	/**
	 * There has GOT to be a better way to get this away message
	 */
    QList<Kopete::ChatSession*> m_chatRoomSessions;


};

class AIMAccount : public OscarAccount
{
Q_OBJECT

public:
	AIMAccount(Kopete::Protocol *parent, QString accountID);
	virtual ~AIMAccount();

	AIMProtocol *protocol() const;

	// fill the menu for this account
	virtual void fillActionMenu( KActionMenu *actionMenu );

	virtual void connectWithPassword( const QString &password );

	void setUserProfile(const QString &profile);
	
	void setPrivacySettings( int privacy );

public slots:
	/** Reimplementation from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage( const Kopete::StatusMessage& statusMessage );
	void slotEditInfo();

	void slotToggleInvisible();

	void slotJoinChat();

protected slots:
    void joinChatDialogClosed( int );

	virtual void loginActions();
	virtual void disconnected( Kopete::Account::DisconnectReason reason );
	virtual void messageReceived( const Oscar::Message& message );

    void connectedToChatRoom( Oscar::WORD exchange, const QString& roomName );
    void userJoinedChat( Oscar::WORD exchange, const QString& room, const QString& contact );
    void userLeftChat( Oscar::WORD exchange, const QString& room, const QString& contact );

protected:

	/**
	* Implement virtual method from OscarAccount
	* This allows OscarAccount to take care of adding new contacts
	*/
	OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const OContact& ssiItem );

private:
	Oscar::Presence presence();

	void setPresenceFlags( Oscar::Presence::Flags flags, const QString &message = QString() );
	void setPresenceType( Oscar::Presence::Type, const QString &awayMessage = QString() );
	void setPresenceTarget( const Oscar::Presence &presence, const QString &message = QString() );

    AIMJoinChatUI* m_joinChatDialog;
	QString mInitialStatusMessage;

	KAction* mJoinChatAction;
	KAction* mEditInfoAction;
	KToggleAction* mActionInvisible;
};
#endif
//kate: tab-width 4; indent-mode csands;
