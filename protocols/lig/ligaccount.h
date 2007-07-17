/*
    ligaccount.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIGACCOUNT_H
#define LIGACCOUNT_H

#include "ligwebcamdialog.h"

#include "kopetepasswordedaccount.h"

#include "ligprotocol.h"

class KActionMenu;
namespace Kopete { class Contact; }
namespace Kopete { class MetaContact; }

class LigContact;
class LigProtocol;
class LigServer;

/**
 * This represents an account connected to the Lig network.
 * LigAccount encapsulates everything that is account-based, as opposed to
 * protocol based. This basically means sockets, current status, and account
 * info are stored in the account, whereas the protocol is only the
 * 'manager' class that creates and manages accounts.
 * @author Cláudio da Silveira Pinheiro
*/
class LigAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	LigAccount( LigProtocol *parent, const QString& accountID, const char *name = 0 );
	~LigAccount();
	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual KActionMenu* actionMenu();

	/**
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplie
	 * Kopete::MetaContact
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);
	/**
	 * Called when Kopete is set globally away
	 */
	virtual void setAway(bool away, const QString& reason);
	/**
	 * Called when Kopete status is changed globally
	 */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const QString &reason = QString::null);
//	/**
//	 * 'Connect' to the lig server.  Only sets myself() online.
//	 */
//	virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus::OnlineStatus() );
//	/**
//	 * Disconnect from the server.  Only sets myself() offline.
//	 */
//	virtual void disconnect();
	/**
	 * Return a reference to the server stub
	 */
	LigServer* server();
	/**
	 * Returns the address of the Lig SIP server
	 */
	QString sipServerName();
	/**
	 * Returns the port of the Lig SIP server
	 */
	uint sipServerPort();
	/**
	 * Returns the address of the Lig STUN server
	 */
	QString stunServerName();
	/**
	 * Returns the port of the Lig STUN server
	 */
	uint stunServerPort();

public slots:
	virtual void connectWithPassword( const QString &password ) ;
	virtual void disconnect() ;
//	virtual void setOnlineStatus( const Kopete::OnlineStatus &status , const QString &reason = QString::null);

	/**
	 * Called by the server when it has a message for us.
	 * This identifies the sending Kopete::Contact and passes it a Kopete::Message
	 */
	void receivedMessage( const QString &message );

protected:
	/**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
	void updateContactStatus();
	LigServer* m_server;

protected slots:
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoAway();
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOffline();
	/**
	 * Show webcam.  Called by KActions and internally.
	 */
	void slotShowVideo();
private:
	QString m_password;
};

#endif
