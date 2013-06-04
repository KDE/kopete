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
 * @brief This Class Represents a Bonjour Account
 *
 * This represents a single Account that broadcasts via the bonjour protocol
 * This has a number of important functions, including:
 * \li running a @ref localServer that listens for incoming connections
 * \li maintaining a list of @ref unknownConnections which are TCP connections where the remote user is unknown
 * \li running a @ref browser that scans for contacts coming online and offline
 * \li publishing our presence via the @ref service
 *
 * @author Tejas Dinkar <tejas\@gja.in>
 */
class BonjourAccount : public Kopete::Account
{
	Q_OBJECT

public:
	/**
	 * @param parent The Parent Protocol
	 * @param accountID A Unique String Identifying this account
	 */
	BonjourAccount( BonjourProtocol *parent, const QString& accountID );
	~BonjourAccount();

	/**
	 * @brief Parse The Config File
	 *
	 * This Function parses the appropriate group in the kopeterc
	 * It sets come internally used constants like @ref username, @ref emailAddress
	 */
	void parseConfig();

	/** 
	 * @brief Verifies a connection is from a claimed user, and return the contact
	 *
	 * This verifies a user by checking that the remote IP address of the connection 
	 * is the same as the user's IP address.
	 *
	 * @param conn The Connection that just discovered it's associated user
	 * @param user A String uniquely identifying the user (username@hotname)
	 * @return The contact if there is a match, or @c NULL otherwise
	 */
	BonjourContact *verifyUser(BonjourContactConnection *conn, const QString &user);

	/**
	 * @brief Get all Contacts at a given Address
	 *
	 * This queries all contacts and returns a list of contacts at a given host address
	 * Usually, there should only be one contact at each address
	 * @param addr The Address to find online contacts at
	 * @return A List of Contacts
	 */
	QList <BonjourContact *> getContactsByAddress(const QHostAddress &addr);

	/**
	 * Construct the context menu used for the status bar icon
	 */
	//virtual KActionMenu* actionMenu();

	/**
	 * @brief Create a New Contact
	 *
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplied
	 * Kopete::MetaContact
	 * This is called internally only, as contacts cannot be added manually
	 *
	 * @return @c true if the contact is created, @c false otherwise
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);

	/**
	 * @brief Called when Kopete is set globally away
	 *
	 * @todo FIXME: This Doesn't Do Anything Right Now
	 */
	virtual void setAway(bool away, const QString& reason);

	/**
	 * @brief Called when Kopete status is changed globally
	 *
	 * @todo FIXME: This Only Makes us go online or offline, we cannot go away
	 */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                             const OnlineStatusOptions& options = None);
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);
	
	/**
	 * @brief 'Connect' to the bonjour service.
	 *
	 * This the one stop call to do everything, like starting server, publishing, discovery, etc
	 * This will clear the contact list in the beginning, and start re populating it
	 * @todo Do something with initialStatus
	 *
	 * @param initialStatus FIXME: This is pretty much ignored
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus() );

	/**
	 * @brief Disconnect from the service.
	 *
	 * This will disconnect from the service. It will stop everything started in connect
	 * It cleans out the contact list after it is finished
	 */
	virtual void disconnect();

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


public slots:

	/**
	 * @brief Slots Called When a Contact Comes Online
	 *
	 * This is Called when A contact Comes online
	 * This is connected to signals from the @ref browser
	 *
	 * @param pointer A RemoteService Pointer to the Service.
	 */
	void comingOnline(DNSSD::RemoteService::Ptr pointer);

	/**
	 * @brief Slots Called When a Contact Goes Offline
	 *
	 * This is Called when A contact goes offline
	 * This is Connected to signals from the @ref browser
	 *
	 * @param pointer A RemoteService Pointer to the Service.
	 */
	void goingOffline(DNSSD::RemoteService::Ptr pointer);

	/**
	 * @brief A Slot Called when a connection discovers it's username
	 *
	 * This is called when a contact connection receives information on who it is connected to
	 * If We don't get this signal (or we can't find a match), then we can try our luck with @ref usernameNotInStream()
	 * Usually, the connection receives the username in a <stream:stream from="username@hostaname"...>
	 * @todo Add A Backup plan
	 *
	 * @param conn The Connection
	 * @param user The User Claimed By the Connection
	 */
	void discoveredUserName(BonjourContactConnection *conn, const QString &user);

	/**
	 * @brief A Slot Called if the connection doesn't get it's username in the stream
	 *
	 * Some IM clients are not decent enough to tell us who they are in the stream (ex: Miranda)
	 * The Expect a lookup of know clients v/s their IP addresses
	 *
	 * @param conn The Connection
	 */
	void usernameNotInStream(BonjourContactConnection *conn);

	/**
	 * @brief This Slot is called when we finish publishing our service
	 *
	 * @param success This is set to true if we successfully started the publish. Else it's an error
	 */
	void published(bool success);

private:

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
	 * The Port it listens on is stored in @ref listeningPort
	 */
	QTcpServer *localServer;

	/**
	 * The port on which @ref localServer is listening
	 */
	int listeningPort;

	/**
	 * The Bonjour Group of The Contact List
	 */
	Kopete::Group *bonjourGroup;

	/**
	 * The Service Browser Which Keeps Scanning For New People
	 */
	DNSSD::ServiceBrowser *browser;

	/*
	 * Big List of Open Connections, who we don't know is at the other end
	 */
	QList <BonjourContactConnection *> unknownConnections;

	/**
	 * @brief The Function To Start the Local Server
	 *
	 * This Start the Local Server. A low numbered port is chosen between 5298 and 5305
	 * The Value of listeningPort is set appropriatly
	 * Remember to set your local firewall to allow us to listen on this port
	 *
	 * @return @c true if the server is now listening, @c false otherwise
	 */
	bool startLocalServer();

	/**
	 * @brief The Function To Start the mDNS Discovery
	 *
	 * This Starts Browsing for broadcasts of _presence._tcp 
	 * All contacts are deleted in the function
	 */
	void startBrowse();

	/**
	 * @brief The Function To Start Advertising the Local Server
	 *
	 * This starts publishing about our @ref localServer
	 * This has an avahi dependency to get our local hostname
	 * @todo FIXME: Remove Avahi Dependency
	 */
	void startPublish();

	/**
	 * @brief This Deletes all the Contacts connected to this account
	 *
	 * This Calls @ref wipeOutContact() internally
	 */
	void wipeOutAllContacts();

	/**
	 * @brief This Deletes a Single Contact
	 *
	 * This deletes a contact. If that is the only contact in the metacontact, it removes that as well
	 * @param c The Contact To Be Deleted
	 */
	void wipeOutContact(Kopete::Contact *c);


protected slots:
	/**
	 * @brief Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	/**
	 * @brief Change the account's status.  Called by KActions and internally.
	 *
	 * @todo FIXME: This Does Nothing Useful
	 */
	void slotGoAway();

	/**
	 * @brief Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOffline();

	/**
	 * @brief This slot is called if a new incoming connection is made
	 *
	 * This is connected to the new connection signal from @ref localServer
	 */
	void newIncomingConnection();

};

#endif
