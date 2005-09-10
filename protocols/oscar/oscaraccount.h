/*
  oscaraccount.h  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARACCOUNT_H
#define OSCARACCOUNT_H

#include <qdict.h>
#include <qstring.h>
#include <qwidget.h>

#include "kopetepasswordedaccount.h"
#include "oscartypeclasses.h"
#include "oscarcontact.h"


namespace Kopete
{
class Contact;
class Group;
}

class Client;
class Connection;
class OscarContact;
class OscarAccountPrivate;

class KDE_EXPORT OscarAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:
	OscarAccount(Kopete::Protocol *parent, const QString &accountID, const char *name=0L, bool isICQ=false);
	virtual ~OscarAccount();

	/** Provide the derived accounts and contacts with access to the backend */
	Client* engine();

	/** Disconnects this account */
	virtual void disconnect();

	/**
	 * Handle the various ways we can be logged off the oscar service
	 * and handle the passthrough of the disconnection through the API.
	 */
	void logOff( Kopete::Account::DisconnectReason );

	/**
	 * Was the password wrong last time we tried to connect?
	 */
	bool passwordWasWrong();

	/**
	 * Sets the account away
	 */
	virtual void setAway( bool away, const QString &awayMessage = QString::null ) = 0;

	/**
	 * Accessor method for the action menu
	 */
	virtual KActionMenu* actionMenu() = 0;

	/** Set the server address */
	void setServerAddress( const QString& server );

	/** Set the server port */
	void setServerPort( int port );

public slots:
	void slotGoOffline();

	void slotGoOnline();

protected:
	/**
	 * Setup a connection for a derived account based on the host and port
	 */
	Connection* setupConnection( const QString& server, uint port );

	/**
	 * Adds a contact to a meta contact
	 */
	virtual bool createContact(const QString &contactId,
		 Kopete::MetaContact *parentContact );

	/**
	 * Protocols using Oscar must implement this to perform the instantiation
	 * of their contact for Kopete.  Called by @ref createContact().
	 * @param contactId theprotocol unique id of the contact
	 * @param parentContact the parent metacontact
	 * @return whether the creation succeeded or not
	 */
	virtual OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem ) = 0;

	virtual QString sanitizedMessage( const QString& message ) = 0;

protected slots:

	//! do stuff on login
	void loginActions();

    void processSSIList();

	void kopeteGroupRemoved( Kopete::Group* g );
	void kopeteGroupAdded( Kopete::Group* g );
	void kopeteGroupRenamed( Kopete::Group* g, const QString& oldName );

	virtual void messageReceived( const Oscar::Message& message );

	void updateContact( Oscar::SSI );

	void ssiGroupAdded( const Oscar::SSI& );
	void ssiGroupRemoved( const Oscar::SSI& ) {}
	void ssiContactAdded( const Oscar::SSI& );
	void ssiContactRemoved( const Oscar::SSI& ) {}

	/* slots for receiving typing notifications, and notify the appropriate OscarContact */
	void userStartedTyping( const QString & contact );
	void userStoppedTyping( const QString & contact );

    void nonServerAddContactDialogClosed();

signals:

	void accountDisconnected( Kopete::Account::DisconnectReason reason );

private:
	QString getFLAPErrorMessage( int code );

private slots:
	/** Handler from socket errors from a connection */
	void slotSocketError( int, const QString& );

	/** Handle task errors from the client */
	void slotTaskError( const Oscar::SNAC& s, int errCode, bool fatal ) ;

private:
	OscarAccountPrivate *d;

};

#endif

//kate: tab-width 4; indent-mode csands;

