/*
    kopeteiface.cpp - Kopete DCOP Interface

    Copyright (c) 2002 by Hendrik vom Lehn       <hennevl@hennevl.de>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteiface.h"
#include "kopetecontactlist.h"
#include "kopetecontact.h"
#include "kopetemessage.h"


KopeteIface::KopeteIface() : DCOPObject( "KopeteIface" )
{
}

QStringList KopeteIface::contacts()
{
	return KopeteContactList::contactList()->contacts();
}

QStringList KopeteIface::reachableContacts()
{
	return KopeteContactList::contactList()->reachableContacts();
}

QStringList KopeteIface::onlineContacts()
{
	QStringList result;
	QPtrList<KopeteContact> list = KopeteContactList::contactList()->onlineContacts();
	QPtrListIterator<KopeteContact> it( list );
	for( ; it.current(); ++it )
		result.append( it.current()->contactId() );

	return result;
}

QStringList KopeteIface::contactsStatus()
{
	return KopeteContactList::contactList()->contactStatuses();
}

QStringList KopeteIface::fileTransferContacts()
{
	return KopeteContactList::contactList()->fileTransferContacts();
}

QStringList KopeteIface::contactFileProtocols(const QString &displayName)
{
	return KopeteContactList::contactList()->contactFileProtocols(displayName);
}

void KopeteIface::messageContact( const QString &displayName, const QString &messageText )
{
	KopeteContactList::contactList()->messageContact( displayName, messageText );
}
/*
void KopeteIface::sendFile(const QString &displayName, const KURL &sourceURL,
	const QString &altFileName = QString::null, const unsigned long fileSize = 0L)
{
	return KopeteContactList::contactList()->sendFile(displayName, sourceURL, altFileName, fileSize);
}


bool KopeteIface::addContact( const QString &protocolName, const QString &contactId,
	const QString &displayName, const QString &groupName )
{
	return KopeteContactList::contactList()->dcopAddContact( protocolName, contactId, displayName, 0L, groupName );
}
*/
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

