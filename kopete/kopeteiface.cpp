/*
    kopeteiface.cpp - Kopete DCOP Interface

    Copyright (c) 2002 by Hendrik vom Lehn       <hennevl@hennevl.de>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

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
#include <kconfig.h>
#include <kglobal.h>

#include "kopeteiface.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteuiglobal.h"
#include "kopeteaway.h"
#include "kopetegroup.h"
#include "kopetecontact.h"

KopeteIface::KopeteIface() : DCOPObject( "KopeteIface" )
{
	KConfig *config = KGlobal::config();
	config->setGroup("AutoAway");

	if (config->readBoolEntry("UseAutoAway", true))
	{
		connectDCOPSignal("kdesktop", "KScreensaverIface",
			"KDE_start_screensaver()", "setAutoAway()", false);
	}
	else
	{
		disconnectDCOPSignal("kdesktop", "KScreensaverIface",
			"KDE_start_screensaver()", "setAutoAway()");
	}
}

QStringList KopeteIface::contacts()
{
	return Kopete::ContactList::contactList()->contacts();
}

QStringList KopeteIface::reachableContacts()
{
	return Kopete::ContactList::contactList()->reachableContacts();
}

QStringList KopeteIface::onlineContacts()
{
	QStringList result;
	QPtrList<Kopete::Contact> list = Kopete::ContactList::contactList()->onlineContacts();
	QPtrListIterator<Kopete::Contact> it( list );
	for( ; it.current(); ++it )
		result.append( it.current()->contactId() );

	return result;
}

QStringList KopeteIface::contactsStatus()
{
	return Kopete::ContactList::contactList()->contactStatuses();
}

QStringList KopeteIface::fileTransferContacts()
{
	return Kopete::ContactList::contactList()->fileTransferContacts();
}

QStringList KopeteIface::contactFileProtocols(const QString &displayName)
{
	return Kopete::ContactList::contactList()->contactFileProtocols(displayName);
}

QString KopeteIface::messageContact( const QString &contactId, const QString &messageText )
{
	Kopete::MetaContact *mc = Kopete::ContactList::contactList()->findMetaContactByContactId( contactId );
	if ( mc && mc->isReachable() )
		Kopete::ContactList::contactList()->messageContact( contactId, messageText );
	else
		return "Unable to send message. The contact is not reachable";
	
	//Default return value
	return QString::null;
}
/*
void KopeteIface::sendFile(const QString &displayName, const KURL &sourceURL,
	const QString &altFileName, uint fileSize)
{
	return Kopete::ContactList::contactList()->sendFile(displayName, sourceURL, altFileName, fileSize);
}

*/

QString KopeteIface::onlineStatus( const QString &metaContactId )
{
	Kopete::MetaContact *m = Kopete::ContactList::contactList()->metaContact( metaContactId );
	if( m )
	{
		Kopete::OnlineStatus status = m->status();
		return status.description();
	}

	return "Unknown Contact";
}

void KopeteIface::messageContactById( const QString &metaContactId )
{
	Kopete::MetaContact *m = Kopete::ContactList::contactList()->metaContact( metaContactId );
	if( m )
	{
		m->execute();
	}
}

bool KopeteIface::addContact( const QString &protocolName, const QString &accountId, const QString &contactId,
	const QString &displayName, const QString &groupName )
{
		//Get the protocol instance
	Kopete::Account *myAccount = Kopete::AccountManager::manager()->findAccount( protocolName, accountId );

	if( myAccount )
	{
		QString contactName;
		QString realGroupName;
		//If the nickName isn't specified we need to display the userId in the prompt
		if( displayName.isEmpty() || displayName.isNull() )
			contactName = contactId;
		else
			contactName = displayName;

		if ( !groupName.isEmpty() )
			realGroupName = groupName;

		// Confirm with the user before we add the contact
		// FIXME: This is completely bogus since the user may not
		// even be at the computer. We just need to add the contact --Matt
		if( KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "An external application is attempting to add the "
			" '%1' contact '%2' to your contact list. Do you want to allow this?" )
			.arg( protocolName ).arg( contactName ), i18n( "Allow Contact?" ) ) == 3 ) // Yes == 3
		{
			//User said Yes
			myAccount->addContact( contactId, contactName, 0L, Kopete::Account::DontChangeKABC, realGroupName, false );
			return true;
		} else {
			//User said No
			return false;
		}

	} else {
		//This protocol is not loaded
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
				 i18n("An external application has attempted to add a contact using "
				      " the %1 protocol, which either does not exist or is not loaded.").arg( protocolName ),
				i18n("Missing Protocol"));

		return false;
	}
}

QStringList KopeteIface::accounts()
{
	QStringList list;
	QPtrList<Kopete::Account> m_accounts=Kopete::AccountManager::manager()->accounts();
	QPtrListIterator<Kopete::Account> it( m_accounts );
	Kopete::Account *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;

		list += ( account->protocol()->pluginId() +"||" + account->accountId() );
	}

	return list;

}

void KopeteIface::connect(const QString &protocolId, const QString &accountId )
{
	QPtrListIterator<Kopete::Account> it( Kopete::AccountManager::manager()->accounts() );
	Kopete::Account *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;

		if( ( account->accountId() == accountId) )
		{
			if( protocolId.isEmpty() || account->protocol()->pluginId() == protocolId )
			{
				account->connect();
				break;
			}
		}
	}
}

void KopeteIface::disconnect(const QString &protocolId, const QString &accountId )
{
	QPtrListIterator<Kopete::Account> it( Kopete::AccountManager::manager()->accounts() );
	Kopete::Account *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;

		if( ( account->accountId() == accountId) )
		{
			if( protocolId.isEmpty() || account->protocol()->pluginId() == protocolId )
			{
				account->disconnect();
				break;
			}
		}
	}
}

void KopeteIface::connectAll()
{
	Kopete::AccountManager::manager()->connectAll();
}

void KopeteIface::disconnectAll()
{
	Kopete::AccountManager::manager()->disconnectAll();
}

bool KopeteIface::loadPlugin( const QString &name )
{
	if ( Kopete::PluginManager::self()->setPluginEnabled( name ) )
	{
		QString argument = name;
		if ( !argument.startsWith( "kopete_" ) )
			argument.prepend( "kopete_" );
		return Kopete::PluginManager::self()->loadPlugin( argument );
	}
	else
	{
		return false;
	}
}

bool KopeteIface::unloadPlugin( const QString &name )
{
	if ( Kopete::PluginManager::self()->setPluginEnabled( name, false ) )
	{
		QString argument = name;
		if ( !argument.startsWith( "kopete_" ) )
			argument.prepend( "kopete_" );
		return Kopete::PluginManager::self()->unloadPlugin( argument );
	}
	else
	{
		return false;
	}
}

void KopeteIface::setAway()
{
	Kopete::AccountManager::manager()->setAwayAll();
}

void KopeteIface::setAway(const QString &msg)
{
	Kopete::AccountManager::manager()->setAwayAll(msg);
}

void KopeteIface::setAvailable()
{
	Kopete::AccountManager::manager()->setAvailableAll();
}

void KopeteIface::setAutoAway()
{
	Kopete::Away::getInstance()->setAutoAway();
}

// vim: set noet ts=4 sts=4 sw=4:

