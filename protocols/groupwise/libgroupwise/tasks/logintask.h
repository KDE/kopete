/*
    Kopete Groupwise Protocol
    logintask.h - Send our credentials to the server and process the contact list and privacy details that it returns.

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#ifndef LOGINTASK_H
#define LOGINTASK_H

#include "requesttask.h"
#include <QByteArray>

using namespace GroupWise;

/**
@author Kopete Developers
*/
class LoginTask : public RequestTask
{
Q_OBJECT
public:
	LoginTask( Task * parent );
	~LoginTask();
	/**
	 * Get the login fields ready to go
	 */
	void initialise();
	/**
	 * Only accepts the contact list that comes back from the server, 
	 * processes it and notifies the client of the contact list
	 */
	bool take( Transfer * transfer );
protected:
	void extractFolder( Field::MultiField * folderContainer );
	void extractContact( Field::MultiField * contactContainer );
	ContactDetails extractUserDetails( Field::FieldList & fields );
	void extractPrivacy( Field::FieldList & fields );
	QStringList readPrivacyItems( const QByteArray & tag, Field::FieldList & fields );
	void extractCustomStatuses( Field::FieldList & fields );
	void extractKeepalivePeriod( Field::FieldList & fields );
signals:
	void gotMyself( const GroupWise::ContactDetails & );
	void gotFolder( const FolderItem & );
	void gotContact( const ContactItem & );
	void gotContactUserDetails( const GroupWise::ContactDetails & );
	void gotPrivacySettings( bool locked, bool defaultDeny, const QStringList & allowList, const QStringList & denyList );
	void gotCustomStatus( const GroupWise::CustomStatus & );
	void gotKeepalivePeriod( int );
};

#endif
