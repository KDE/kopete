/***************************************************************************
                          jabberresource.cpp  -  description
                             -------------------
    begin                : Thu Aug 15 2002
    copyright            : (C) 2002 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatetime.h>
#include <kdebug.h>
#include <psi/types.h>
#include "jabberprotocol.h"
#include "jabberresource.h"

JabberResource::JabberResource()
{

	kdDebug() << "Jabber resource: New Jabber resource (no params)." << endl;

}

JabberResource::JabberResource(const QString &resource, const int &priority, const QDateTime &timestamp, const JabberProtocol::Presence &status, const QString &reason)
{
	
	kdDebug() << QString("Jabber resource: New Jabber resource (resource %1, priority %2, timestamp %3).").arg(resource, 1).arg(priority, 2).arg(timestamp.toString("yyyyMMddhhmmss"), 3) << endl;
	
	mResource = resource;
	mPriority = priority;
	mTimestamp = timestamp;
	mStatus = status;
	mReason = reason;

}

JabberResource::~JabberResource()
{

}

QString JabberResource::resource()
{
    return mResource;
}
		
int JabberResource::priority()
{
    return mPriority;
}

QDateTime JabberResource::timestamp()
{
    return mTimestamp;
}
	
JabberProtocol::Presence JabberResource::status()
{
    return mStatus;
}

QString JabberResource::reason()
{
    return mReason;
}

#include "jabberresource.moc"
