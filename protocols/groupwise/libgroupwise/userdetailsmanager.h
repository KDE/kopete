/*
    userdetailsmanager.h - Storage of all user details seen during this session
   
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef USERDETAILSMANAGER_H
#define USERDETAILSMANAGER_H

#include <QMap>
#include <QObject>
#include <QStringList>

#include "gwerror.h"
#include "libgroupwise_export.h"
#include "client.h"

namespace GroupWise {
	class Client;
}

/**
Several client event handling processes require that a contact's details are available before exposing the event to the user.  This class is responsible for issuing details requests, tracking which users the client already has received details for, and signalling when details have been received.  The manager allows multiple interleaved get details requests to be replaced by a single request.

@author SUSE AG
*/

class LIBGROUPWISE_EXPORT UserDetailsManager : public QObject
{
Q_OBJECT
public:
	UserDetailsManager( Client * parent);
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
	 * Get details for a given DN
	 */
	ContactDetails details( const QString &dn );
	/** 
	 * Add a ContactDetails object to our cache.
	 * This SHOULD be called when receiving details in contact list receive and manipulation, to prevent unnecessary additional requests.
	 */
	void addDetails( const GroupWise::ContactDetails & details );
	/**
	 * Remove a contact from the list of known DNs.  This MUST be performed when a client removes a DN from its local contact list,
	 * otherwise new events from this DN will not receive user details.
	 */
	void removeContact( const QString & dn );
	/**
	 * Explicitly request details for a set of contacts from the server.
	 * Will signal @ref gotContactUserDetails for each one when they are available.
	 */
	void requestDetails( const QStringList & dnList, bool onlyUnknown = true );
	/**
	 * Explicitly request a contact's details from the server.  Will signal @ref gotContactUserDetails when they are available.
	 */
	void requestDetails( const QString & dn, bool onlyUnknown = true );
	
signals:
	void gotContactDetails( const GroupWise::ContactDetails & );
protected slots:
	void slotReceiveContactDetails( const GroupWise::ContactDetails & );
protected:
	void dump( const QStringList & list );
private:
	QStringList m_pendingDNs;	// a list of DNs that have pending requests
	Client * m_client;
	QMap< QString, GroupWise::ContactDetails > m_detailsMap;
};

#endif
