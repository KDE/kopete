/*
    gwaccount.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopetemetacontact.h"

#include "gwcontact.h"
#include "gwfakeserver.h"
#include "gwprotocol.h"


GroupWiseAccount::GroupWiseAccount( GroupWiseProtocol *parent, const QString& accountID, const char *name )
: KopeteAccount ( parent, accountID , name )
{
	// Init the myself contact
	// FIXME: I think we should add a global self metaContact (Olivier)
	setMyself( new GroupWiseContact( this, accountId(), GroupWiseContact::Null, accountId(), 0L ) );
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
	m_server = new GroupWiseFakeServer();;
}

GroupWiseAccount::~GroupWiseAccount()
{
	delete m_server;
}

KActionMenu* GroupWiseAccount::actionMenu()
{
	KActionMenu *theActionMenu = new KActionMenu(accountId(), myself()->onlineStatus().iconFor(this) , this);
	theActionMenu->popupMenu()->insertTitle(myself()->icon(), i18n("GroupWise (%1)").arg(accountId()));
	// NEED FSCKING GO ONLINE OFFLINE ACTIONS HERE!
	theActionMenu->insert(new KAction (GroupWiseProtocol::protocol()->groupwiseOnline.caption(),
		GroupWiseProtocol::protocol()->groupwiseOnline.iconFor(this), 0, this, SLOT (slotGoOnline ()), this,
		"actionGroupWiseConnect"));

	theActionMenu->insert(new KAction (GroupWiseProtocol::protocol()->groupwiseAway.caption(),
		GroupWiseProtocol::protocol()->groupwiseAway.iconFor(this), 0, this, SLOT (slotGoAway ()), this,
		"actionGroupWiseAway"));

	theActionMenu->insert(new KAction (GroupWiseProtocol::protocol()->groupwiseOffline.caption(),
		GroupWiseProtocol::protocol()->groupwiseOffline.iconFor(this), 0, this, SLOT (slotGoOffline ()), this,
		"actionGroupWiseOfflineDisconnect"));
	
	return theActionMenu;
}

bool GroupWiseAccount::addContactToMetaContact(const QString& contactId, const QString& displayName, KopeteMetaContact* parentContact)
{
	kdDebug ( 14210 ) << k_funcinfo << "contactId: " << contactId << " displayName: " << displayName
			<< endl;
	GroupWiseContact* newContact = new GroupWiseContact( this, contactId, GroupWiseContact::Echo, displayName, parentContact );
	return newContact != 0L;
}

void GroupWiseAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void GroupWiseAccount::connect()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOnline );
	QObject::connect ( m_server, SIGNAL ( messageReceived( const QString & ) ),
			this, SLOT ( receivedMessage( const QString & ) ) );
}

void GroupWiseAccount::disconnect()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

GroupWiseFakeServer * GroupWiseAccount::server()
{
	return m_server;
}

void GroupWiseAccount::slotGoOnline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseOnline );
	updateContactStatus();
}
void GroupWiseAccount::slotGoAway ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();
	
	myself()->setOnlineStatus( GroupWiseProtocol::protocol()->groupwiseAway );
	updateContactStatus();
}


void GroupWiseAccount::slotGoOffline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void GroupWiseAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	GroupWiseContact* messageSender;
	
	from = message.section( ':', 0, 0 );
	//from = QString::fromLatin1("echo");
	messageSender = static_cast<GroupWiseContact *>( contacts ()[ from ] );
	
	kdDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	messageSender->receivedMessage( message );
}

void GroupWiseAccount::updateContactStatus()
{
	QDictIterator<KopeteContact> itr( contacts() );
	for ( ; itr.current(); ++itr )
		itr.current()->setOnlineStatus( myself()->onlineStatus() );
}


#include "gwaccount.moc"
