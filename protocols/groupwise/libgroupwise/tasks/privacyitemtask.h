/*
    Kopete Groupwise Protocol
    privacyitemtask.h - Add an entry to the server side deny or allow lists

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

#ifndef PRIVACYITEMTASK_H
#define PRIVACYITEMTASK_H

#include "requesttask.h"

/**
Adds a contact to the server side allow or deny lists

@author SUSE AG
*/
class PrivacyItemTask : public RequestTask
{
Q_OBJECT
public:
	PrivacyItemTask( Task* parent);
	~PrivacyItemTask();
	void allow( const QString & dn );
	void deny( const QString & dn );
	void removeAllow( const QString & dn );
	void removeDeny( const QString & dn );
	void defaultPolicy( bool defaultDeny );
	QString dn() const;
	bool defaultDeny() const;
	// void contacts( const QStringList & contacts );
private:
	bool m_default;
	QString m_dn;
};

#endif
