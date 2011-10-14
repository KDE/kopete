/*
    Kopete Groupwise Protocol
    privacymanager.cpp - stores the user's privacy information and maintains it on the server

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

#ifndef PRIVACYMANAGER_H
#define PRIVACYMANAGER_H

#include <QObject>
#include <QStringList>
#include "libgroupwise_export.h"

namespace GroupWise {
	class Client;
}

/**
Keeps a record of the server side privacy allow and deny lists, default policy and whether the user is allowed to change privacy settings

@author SUSE AG
*/
class LIBGROUPWISE_EXPORT PrivacyManager : public QObject
{
Q_OBJECT
public:
	PrivacyManager( GroupWise::Client * client);
	~PrivacyManager();
	// accessors
	bool isBlocked( const QString & dn );
	QStringList allowList();
	QStringList denyList();
	bool isPrivacyLocked();
	bool defaultDeny();
	bool defaultAllow();
	// mutators
	void setDefaultAllow( bool allow );
	void setDefaultDeny( bool deny );
	void setAllow( const QString & dn );
	void setDeny( const QString & dn );
	void getDetailsForPrivacyLists();
	// change everything at once
	void setPrivacy( bool defaultIsDeny, const QStringList & allowList, const QStringList & denyList );

signals:
	void privacyChanged( const QString &dn, bool allowed );
public slots:
	/** 
	 * Used to initialise the privacy manager using the server side privacy list
	 */
	void slotGotPrivacySettings( bool locked, bool defaultDeny, const QStringList & allowList, const QStringList & denyList );
protected:
	void addAllow( const QString & dn );
	void addDeny( const QString & dn );
	void removeAllow( const QString & dn );
	void removeDeny( const QString & dn );
	/**
	 * A set difference function
	 * @param lhs The set of strings to be subtracted from
	 * @param rhs The set of string to subtract
	 * @return The difference between the two sets
	 */
	QStringList difference( const QStringList & lhs, const QStringList & rhs );
protected slots:
	// Receive the results of Tasks manipulating the privacy lists
	void slotDefaultPolicyChanged();
	void slotAllowAdded();
	void slotDenyAdded();
	void slotAllowRemoved();
	void slotDenyRemoved();	
private:
	GroupWise::Client * m_client;
	bool m_locked;
	bool m_defaultDeny;
	QStringList m_allowList;
	QStringList m_denyList;
};

#endif
