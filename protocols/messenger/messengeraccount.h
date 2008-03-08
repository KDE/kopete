/*
 * messengeraccount.h - Windows Live Messenger Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef MESSENGERACCOUNT_H
#define MESSENGERACCOUNT_H

#include <kopetepasswordedaccount.h>
#include "messengerprotocol.h"
#include "kaction.h"
#include "Papillon/Client"
#include "Papillon/ContactList"
#include "Papillon/QtConnector"
#include <QStringList>

#define MESSENGER_DEFAULT_PORT	1863
#define MESSENGER_DEFAULT_SERVER	"muser.messenger.hotmail.com"

class KActionMenu;

namespace Papillon
{
    class Base;
    class Contact;
    class Client;
    class Global;
    class Presence;
    class ContactList;
    class QtConnector;
}
namespace Kopete 
{ 
	class Account;
	class MetaContact;
	class StatusMessage;
}

class MessengerProtocol;

class MessengerAccountPrivate;

class MessengerAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	MessengerAccount(MessengerProtocol *protocol, const QString &accountId);
	~MessengerAccount();

	/* Fills the menu for this account. */
	//virtual void fillActionMenu( KActionMenu *actionMenu );

	/* Return the resource of the client */
	QString serverName () const;
	int serverPort () const;

	/* to get the protocol from the account */
	MessengerProtocol *protocol() const
	{
		return m_protocol;
	}

	Papillon::Client *client () const
	{
		return m_messengerClient;
	}

	/* Tells the user to connect first before they can do whatever it is
	 * that they want to do. */
	void errorConnectFirst ();

	/* Tells the user that the connection was lost while we waited for
	 * an answer of him. */
	void errorConnectionLost ();

	/*
	 * called when the account is removed in the config ui */
	virtual bool removedAccount();

	 /*
	 * picture */
	QString pictureUrl();
	void setPictureUrl(const QString &url);
	QString pictureObject();
	void resetPictureObject(bool silent=false);


	bool useHttpMethod() const;
	QString myselfClientId() const;
	void setPublicName( const QString &publicName );

	//Actions
	KAction * m_openInboxAction;
	KAction * m_changeDNAction;
	KAction * m_editUserInfoAction;
	KAction * m_startChatAction;

	Kopete::OnlineStatus m_connectstatus;
	QStringList m_msgHandle;
	bool m_newContactList;
	uint m_clientId;

	// server data
	QList<Papillon::Contact*> m_contactList;
	QList<Papillon::Contact*> m_allowList;
	QList<Papillon::Contact*> m_blockList;
	QList<Papillon::Contact*> m_reverseList;

public slots:
	
	/* Connects to the server. */
	void connectWithPassword(const QString &password);
	/* Disconnects from the server. */
	void disconnect();
	/* Disconnect with a reason */
	//void disconnect ( Kopete::Account::DisconnectReason reason );
	/* Disconnect with a reason, and status */
	//void disconnect( Kopete::Account::DisconnectReason reason, XMPP::Status &status );
	/* Reimplemented from Kopete::Account */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage());

	virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);


	void slotChangePublicName();
	void slotStartChat();
private slots:
      /**
	 * When the dispatch server sends us the notification server to use.
	 */
	void createNotificationServer( const QString &host, uint port );
protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact);
	
private:
	void cleanup ();

	MessengerProtocol *m_protocol;
	// backend for this account
	QString m_pictureObj; //a cache of the <msnobj>
	QString m_pictureFilename; // the picture filename.
	Papillon::Client * m_messengerClient;

	/* Initial presence to set after connecting. */
	//Papillon::Presence::Status m_initialPresence;
};
#endif
