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

#include <kmessagebox.h>
#include <klocale.h>

#include "kopeteiface.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"


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
	const QString &altFileName, uint fileSize)
{
	return KopeteContactList::contactList()->sendFile(displayName, sourceURL, altFileName, fileSize);
}

*/
bool KopeteIface::addContact( const QString &protocolName, const QString &accountId, const QString &contactId,
	const QString &displayName, const QString &groupName )
{
		//Get the protocol instance
	KopeteAccount *myAccount = KopeteAccountManager::manager()->findAccount( protocolName, accountId );

	if( myAccount )
	{
		QString contactName;

		//If the nickName isn't specified we need to display the userId in the prompt
		if( displayName.isEmpty() || displayName.isNull() )
			contactName = contactId;
		else
			contactName = displayName;

		// Confirm with the user before we add the contact
		if( KMessageBox::questionYesNo( 0, i18n( "An external application is attempting to add the "
			" '%1' contact '%2' to your contact list. Do you want to allow this?" )
			.arg( protocolName ).arg( contactName ), i18n( "Allow Contact?" ) ) == 3 ) // Yes == 3
		{
			//User said Yes
			myAccount->addContact( contactId, displayName, 0L, groupName, false );
			return true;
		} else {
			//User said No
			return false;
		}

	} else {
		//This protocol is not loaded
		KMessageBox::error( 0, i18n("An external application has attempted to add a contact using "
				" the %1 protocol, which either does not exist or is not loaded.").arg( protocolName ),
				i18n("Missing Protocol"));

		return false;
	}
}


// vim: set noet ts=4 sts=4 sw=4:

