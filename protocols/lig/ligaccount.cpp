/*
    ligaccount.cpp - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "ligaccount.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kconfig.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetepassword.h"

#include "ligcontact.h"
#include "ligserver.h"
#include "ligprotocol.h"

LigAccount::LigAccount( LigProtocol *parent, const QString& accountID, const char *name )
: Kopete::PasswordedAccount ( parent, accountID.lower() , 0, name )
{
	// Init the myself contact
	setMyself( new LigContact( this, accountId(), LigContact::Null, accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( LigProtocol::protocol()->ligOffline );
	m_server = new LigServer();;

	KConfigGroup *config=configGroup();
}

LigAccount::~LigAccount()
{
	delete m_server;
}

KActionMenu* LigAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->popupMenu()->insertSeparator();

	KAction *action;
	
	action = new KAction (i18n ("Show my own video..."), "lig_showvideo", 0, this, SLOT (slotShowVideo ()), this, "actionShowVideo");
	mActionMenu->insert(action);
	action->setEnabled( isConnected() );

	return mActionMenu;
}

bool LigAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	LigContact* newContact = new LigContact( this, contactId, LigContact::Echo, parentContact->displayName(), parentContact );
	return newContact != 0L;
}

void LigAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void LigAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const QString &reason )
{
	if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		slotGoOnline();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
		setAway( false, reason );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		slotGoOffline();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( /* reason */ );
}

//void LigAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
/*{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( LigProtocol::protocol()->ligOnline );
	QObject::connect ( m_server, SIGNAL ( messageReceived( const QString & ) ),
			this, SLOT ( receivedMessage( const QString & ) ) );
}*/

void LigAccount::connectWithPassword( const QString &passwd )
{
//	m_newContactList=false;
	if ( isConnected() )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Ignoring Connect request "
			<< "(Already Connected)" << endl;
		return;
	}

//	if ( m_notifySocket )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Ignoring Connect request (Already connecting)"  << endl;
		return;
	}

	m_password = passwd;

//	if ( m_password.isNull() )
	{
		kdDebug( 14140 ) << k_funcinfo <<"Abort connection (null password)"  << endl;
		return;
	}


	if ( contacts().count() <= 1 )
	{
		// Maybe the contactlist.xml has been removed, and the serial number not updated
		// ( the 1 is for the myself contact )
		configGroup()->writeEntry( "serial", 0 );
	}

//	m_openInboxAction->setEnabled( false );

//	createNotificationServer(sipServerName(), sipServerPort());
}

void LigAccount::disconnect()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;
	myself()->setOnlineStatus( LigProtocol::protocol()->ligOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

LigServer * LigAccount::server()
{
	return m_server;
}

void LigAccount::slotGoOnline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( LigProtocol::protocol()->ligOnline );
	updateContactStatus();
}

void LigAccount::slotGoAway ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (!isConnected ())
		connect();

	myself()->setOnlineStatus( LigProtocol::protocol()->ligAway );
	updateContactStatus();
}


void LigAccount::slotGoOffline ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void LigAccount::slotShowVideo ()
{
	kdDebug ( 14210 ) << k_funcinfo << endl;

	if (isConnected ())
		LigWebcamDialog *ligWebcamDialog = new LigWebcamDialog(0, 0, "Lig video window");
	updateContactStatus();
}

void LigAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	LigContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts()[from];
	messageSender = dynamic_cast<LigContact *>( contact );

	kdDebug( 14210 ) << k_funcinfo << " got a message from " << from << ", " << messageSender << ", is: " << message << endl;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		messageSender->receivedMessage( message );
	else
		kdWarning(14210) << k_funcinfo << "unable to look up contact for delivery" << endl;
}

void LigAccount::updateContactStatus()
{
	QDictIterator<Kopete::Contact> itr( contacts() );
	for ( ; itr.current(); ++itr )
		itr.current()->setOnlineStatus( myself()->onlineStatus() );
}

QString LigAccount::sipServerName()
{
	return configGroup()->readEntry(  "sipServerName" , "sip.melig.com.br" );
}

uint LigAccount::sipServerPort()
{
	return configGroup()->readNumEntry(  "sipServerPort" , 5060 );
}

QString LigAccount::stunServerName()
{
	return configGroup()->readEntry(  "stunServerName" , "stun.melig.com.br" );
}

uint LigAccount::stunServerPort()
{
	return configGroup()->readNumEntry(  "stunServerPort" , 3478 );
}


#include "ligaccount.moc"
