/*
  oscaraccount.h  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qstring.h>
#include <qwidget.h>

#include "kopetepasswordedaccount.h"
#include "oscartypeclasses.h"
#include "oscarcontact.h"
#include "contact.h"
#include "kopete_export.h"

class QDomNode;

namespace Kopete
{
class Contact;
class Group;
class FileTransferInfo;
}

namespace Oscar {
class Client;
}
class ClientStream;
class OscarContact;
class OscarAccountPrivate;
class QTextCodec;
class OContact;

class OSCAR_EXPORT OscarAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:
	OscarAccount(Kopete::Protocol *parent, const QString &accountID, bool isICQ=false);
	virtual ~OscarAccount();

	/** Provide the derived accounts and contacts with access to the backend */
	Oscar::Client* engine();

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
	 * Fill the menu for this account
	 */
	virtual void fillActionMenu( KActionMenu *actionMenu ) = 0;

	/**
	 * Sets the identity this account belongs to
	 */
	virtual bool setIdentity( Kopete::Identity *ident );

	/** Set the server address */
	void setServerAddress( const QString& server );

	/** Set the server port */
	void setServerPort( int port );

	/** Set the server encryption */
	void setServerEncrypted( bool encrypted );

    /** Set if the proxy server is SOCKS5 */
    void setProxyServerSocks5( bool enable );

    /** Set the proxy server address */
    void setProxyServerAddress( const QString& server );

    /** Set the proxy server port */
    void setProxyServerPort( int port );

    void setProxyServerEnabled(bool);

	/** Returns codec for account's default encoding */
	QTextCodec* defaultCodec() const;

	/**
	 * Returns codec for given contact's encoding or default one
	 * if contact has no encoding
	 */
	QTextCodec* contactCodec( const OscarContact* contact ) const;

	/**
	 * Returns codec for given contact's encoding or default one
	 * if contact has no encoding
	 */
	QTextCodec* contactCodec( const QString& contactName ) const;

	/**
	 * Updates buddy icon
	 */
	void updateBuddyIcon( const QString &path );

	/**
	 * Add a contact to the server site list
	 * \param contactName the screen name of the new contact to add
	 * \param groupName the group of the new contact
	 * \param autoAddGroup if the group doesn't exist add that group
	 * \return true if the contact will be added
	 */
	bool addContactToSSI( const QString& contactName, const QString& groupName, bool autoAddGroup );

	/**
	 * Change a contact's group on the server
	 * \param contact the contact to change
	 * \param newGroup the new group to move the contact to
	 * \param autoAddGroup if the new group doesn't exist add that group
	 * \return true if the contact will be added
	 */
	bool changeContactGroupInSSI( const QString& contact, const QString& newGroupName, bool autoAddGroup );

public slots:
	void slotGoOffline();

	void slotGoOnline();

protected:
	/**
	 * Adds a contact to a meta contact
	 */
	friend class OscarProtocol;
	virtual bool createContact(const QString &contactId,
		 Kopete::MetaContact *parentContact );

	/**
	 * Protocols using Oscar must implement this to perform the instantiation
	 * of their contact for Kopete.  Called by @ref createContact().
	 * @param contactId theprotocol unique id of the contact
	 * @param parentContact the parent metacontact
	 * @return whether the creation succeeded or not
	 */
	virtual OscarContact *createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const OContact& ssiItem ) = 0;

	void updateVersionUpdaterStamp();

	virtual QString sanitizedMessage( const QString& message ) const;

protected slots:

	//! do stuff on login
	virtual void loginActions();

    void processSSIList();

	void kopeteGroupRemoved( Kopete::Group* g );
	void kopeteGroupAdded( Kopete::Group* g );
	void kopeteGroupRenamed( Kopete::Group* g, const QString& oldName );

	virtual void messageReceived( const Oscar::Message& message );

	void ssiGroupAdded( const OContact& );
	void ssiGroupUpdated( const OContact& ) {}
	void ssiGroupRemoved( const OContact& ) {}
	void ssiContactAdded( const OContact& );
	void ssiContactUpdated( const OContact& );
	void ssiContactRemoved( const OContact& ) {}

	/* slots for receiving typing notifications, and notify the appropriate OscarContact */
	void userStartedTyping( const QString & contact );
	void userStoppedTyping( const QString & contact );

    void nonServerAddContactDialogClosed();

	/** incoming filetransfer */
	void incomingFileTransfer( FileTransferHandler* handler );

	void fileTransferDestroyed( QObject* handler );
	void fileTransferCancelled();
	void fileTransferRefused( const Kopete::FileTransferInfo& info );
	void fileTransferAccept( Kopete::Transfer* t , const QString& fileName );

	void chatroomRequest( ChatRoomHandler* handler );

signals:

	void accountDisconnected( Kopete::Account::DisconnectReason reason );

private:
	QString getFLAPErrorMessage( int code );

	/** Updates buddy icon item in ssi */
	void updateBuddyIconInSSI();

	QString makeWellFormedXML( const QString& message ) const;

	QString addQuotesAroundAttributes( QString message ) const;

	QString sanitizedPlainMessage( const QString& message ) const;

	QList<QDomNode> getElementsByTagNameCI( const QDomNode& node, const QString& tagName ) const;

private slots:
	/** Handler from socket errors from a connection */
	void slotSocketError( int, const QString& );

	/** Handle task errors from the client */
	void slotTaskError( const Oscar::SNAC& s, int errCode, bool fatal ) ;

	/** Sends buddy icon to server */
	void slotSendBuddyIcon();

	/** Identity's property changed */
	void slotIdentityPropertyChanged( Kopete::PropertyContainer *container, const QString &key,
	                                  const QVariant &oldValue, const QVariant &newValue );

	/**
	 * Setup a ClientStream for engine
	 */
	void createClientStream( ClientStream **clientStream );

private:
	OscarAccountPrivate *d;

};

#endif

//kate: tab-width 4; indent-mode csands;

