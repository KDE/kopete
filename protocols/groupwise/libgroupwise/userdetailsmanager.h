//
// C++ Interface: userdetailsmanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef USERDETAILSMANAGER_H
#define USERDETAILSMANAGER_H

#include <qobject.h>
#include <qstringlist.h>

#include "gwerror.h"
class Client;

/**
Several client event handling processes require that a contact's details are available before exposing the event to the user.  This class is responsible for issuing details requests, tracking which users the client already has received details for, and signalling when details have been received.  The manager allows multiple interleaved get details requests to be replaced by a single request.

@author SUSE AG
*/
class UserDetailsManager : public QObject
{
Q_OBJECT
public:
	UserDetailsManager( Client * parent, const char *name = 0);
	~UserDetailsManager();
	/**
	 * List of DNs that we have already received details for
	 */
	QStringList knownDNs();
	/**
	 * Check if we have details for a single DN
	 */
	bool known( const QString &dn );
	/** 
	 * Add a DN to the list of DNs that we have details for.  This SHOULD be called when receiving details in contactlist receive and manipulation, to prevent unnecessary additional requests.
	 */
	void addContact( const QString &dn );
	/**
	 * Remove a contact from the list of known DNs.  This MUST be performed when a client removes a DN from its local contact list,
	 * otherwise new events from this DN will not receive user details.
	 */
	void removeContact( const QString & dn );
	/**
	 * Explicitly request details for a set of contacts from the server.
	 * Will signal @ref gotContactUserDetails for each one when they are available.
	 */
	void requestDetails( const QStringList & dnList );
	/**
	 * Explicitly request a contact's details from the server.  Will signal @ref gotContactUserDetails when they are available.
	 */
	void requestDetails( const QString & dn );
	
signals:
	void temporaryContact( const ContactDetails & );
	void gotContactDetails( const GroupWise::ContactDetails & );
protected slots:
	void slotReceiveContactDetails( const GroupWise::ContactDetails & );
private:
	QStringList m_knownDNs; 	// a list of DNs that we have details for already
	QStringList m_pendingDNs;	// a list of DNs that have pending requests
	Client * m_client;
};

#endif
