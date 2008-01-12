/*
    bonjouraccount.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef BONJOURACCOUNT_H
#define BONJOURACCOUNT_H

#include <QByteArray>
#include <QTcpServer>
#include <QList>

#include <kopeteaccount.h>

#include <dnssd/publicservice.h>
#include <dnssd/servicebrowser.h>

#include "bonjourcontact.h"
#include "bonjourwebcamdialog.h"
#include "bonjourcontactconnection.h"

class KActionMenu;
namespace Kopete 
{ 
	class Contact;
	class MetaContact;
	class StatusMessage;
}

class BonjourProtocol;

/**
 * This represents an account connected to the bonjour
 * @author Tejas Dinkar
*/
class BonjourAccount : public Kopete::Account
{
	Q_OBJECT

	/** 
	 * The Following Details are Set When An Account is Created
	 */
	QByteArray username;
	QByteArray firstName; 
	QByteArray emailAddress; 
	QByteArray lastName; 


	/**
	 * The Connection to Avahi Which Keeps Scanning For Contacts Coming Online Or Offline
	 */
	DNSSD::PublicService *service;

	/**
	 * The local Server which waits for people to talk to it :D
	 * The Port it listens on is stored in an integer
	 */
	QTcpServer *localServer;
	int listeningPort;

	/**
	 * The Service Browser Which Keeps Scanning For New People
	 */
	DNSSD::ServiceBrowser *browser;

	/**
	 * The Function To Start the Local Server
	 */
	bool startLocalServer();

	/**
	 * The Function To Start the Discovery
	 */
	void startBrowse();

	/**
	 * The Function To Start Advertising the Local Server
	 */
	void startPublish();

	/**
	 * The Bonjour Group of The Contact List
	 */
	Kopete::Group *bonjourGroup;

	/*
	 * Wipe Out Contacts, Either One By One, or all at once
	 */
	void wipeOutContact(Kopete::Contact *c);
	void wipeOutAllContacts();

	/*
	 * Big List of Open Connections, who we don't know is at the other end
	 */
	QList <BonjourContactConnection *> unknownConnections;

public:
	BonjourAccount( BonjourProtocol *parent, const QString& accountID );
	~BonjourAccount();

	/*
	 * Parse The Config File
	 */
	void parseConfig();

	/**
	 * Set Properties Such As username, firstName, emailAddress, lastName
	 */

	Q_PROPERTY(QByteArray username READ getusername WRITE setusername)
	Q_PROPERTY(QByteArray firstName READ getfirstName WRITE setfirstName)
	Q_PROPERTY(QByteArray lastName READ getlastName WRITE setlastName)
	Q_PROPERTY(QByteArray emailAddress READ getemailAddress WRITE setemailAddress)

	void setusername(const QByteArray &nusername);
	void setfirstName(const QByteArray &nfirstName);
	void setlastName(const QByteArray &nlastName);
	void setemailAddress(const QByteArray &nemailAddress);

	const QByteArray getusername() const;
	const QByteArray getfirstName() const;
	const QByteArray getlastName() const;
	const QByteArray getemailAddress() const;

	/**
	 * Resolve A Hostname Via Avahi
	 * FIXME: Remove Avahi Dependency
	 */
	static QString resolveHostName(const QString &hostName);
	static QString getLocalHostName();

	/* Verify A User's IP Address Here
	 * FIXME: For now it just verifies if the contact exists
	 */
	BonjourContact *verifyUser(BonjourContactConnection *conn, QString user);

	/**
	 * Return all Contacts at a given Address (usually 1)
	 */
	QList <BonjourContact *> getContactsByAddress(const QHostAddress &addr);

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
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage() );
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);
	/**
	 * 'Connect' to the bonjour server.  Only sets myself() online.
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus::OnlineStatus() );
	/**
	 * Disconnect from the server.  Only sets myself() offline.
	 */
	virtual void disconnect();

public slots:
	/**
	 * Called by the server when it has a message for us.
	 * This identifies the sending Kopete::Contact and passes it a Kopete::Message
	 */
	void receivedMessage( const QString &message );

	/**
	 * These are Called when A contact Comes online or offline
	 * These are connected to signals from the browser
	 */
	void comingOnline(DNSSD::RemoteService::Ptr pointer);
	void goingOffline(DNSSD::RemoteService::Ptr pointer);

	/**
	 * This is called when a contact connection receives information on who it is connected to
	 */
	void discoveredUserName(BonjourContactConnection *conn, QString user);

protected:
	/**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
	void updateContactStatus();


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

	/**
	 * This slot is called if a new incomin connection is made
	 */
	void newIncomingConnection();

};

#endif
