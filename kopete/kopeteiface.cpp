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
	return KopeteContactList::contactList()->onlineContacts();
}

QStringList KopeteIface::contactsStatus()
{
	return KopeteContactList::contactList()->contactStatuses();
}

QStringList KopeteIface::fileTransferContacts()
{
	return KopeteContactList::contactList()->fileTransferContacts();
}

void KopeteIface::sendFile(QString displayName, QString fileName)
{
	return KopeteContactList::contactList()->sendFile(displayName, fileName);
}



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

