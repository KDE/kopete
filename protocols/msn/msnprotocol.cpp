/***************************************************************************
                          msnprotocol.cpp  -  MSN Plugin
                             -------------------
    Copyright (c) 2002        by Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2002        by Martijn Klingens    <klingens@kde.org>
    Copyright (c) 2002-2003   by Olivier Goffart     <ogoffart@tiscalinet.be>

    Copyright (c) 2002-2003  by the Kopete developers  <kopete-devel@kde.org>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qapplication.h>
#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kopetecontactlist.h"
#include "kopeteidentitymanager.h"
#include "kopetemetacontact.h"

#include "msnaddcontactpage.h"
#include "msneditidentitywidget.h"
#include "msncontact.h"
#include "msnidentity.h"
#include "msnpreferences.h"
#include "msnprotocol.h"
#include "msnmessagemanager.h"

K_EXPORT_COMPONENT_FACTORY( kopete_msn, KGenericFactory<MSNProtocol> );

MSNProtocol::MSNProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	QString protocolId = pluginId();

	// Go in experimental mode: enable the new API :-)
	//enableStreaming( true );

	kdDebug(14140) << "MSNProtocol::MSNProtocol: MSN Plugin Loading" << endl;

	mPrefs= new MSNPreferences( "msn_protocol", this );

	m_menu=0L;

	setStatusIcon( "msn_offline" );

	addAddressBookField( "messaging/msn", KopetePlugin::MakeIndexField );
}

MSNProtocol::~MSNProtocol()
{
}

void MSNProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId   = serializedData[ "contactId" ];
	QString identityId = serializedData[ "identityId" ];
	QString displayName = serializedData[ "displayName" ];
	QStringList groups  = QStringList::split( ",", serializedData[ "groups" ] );	
	
	QDict<KopeteIdentity> identities=KopeteIdentityManager::manager()->identities(this);
	
	if(identityId.isNull())
	{
		//Kopete 0.6.x contactlist
		KGlobal::config()->setGroup("MSN");
		identityId=KGlobal::config()->readEntry( "UserID", "" );;
	}
	KopeteIdentity *identity=identities[identityId];
	if(!identity)
	{
		identity=createNewIdentity(identityId);
	}

	// Create MSN contact
	MSNContact *c = new MSNContact( identity, contactId, displayName, metaContact );
	c->setMsnStatus( MSNProtocol::FLN );
	for( QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
		c->contactAddedToGroup( ( *it ).toUInt(), 0L  /* FIXME - m_groupList[ ( *it ).toUInt() ]*/ );
}

AddContactPage *MSNProtocol::createAddContactWidget(QWidget *parent)
{
	return (new MSNAddContactPage(this,parent));
}

EditIdentityWidget *MSNProtocol::createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent)
{
	return new MSNEditIdentityWidget(this,identity,parent);
}

KopeteIdentity *MSNProtocol::createNewIdentity(const QString &identityId)
{
	return new MSNIdentity(this, identityId);
}

KActionMenu* MSNProtocol::protocolActions()
{
	delete m_menu;
	m_menu=0L;
	QDict<KopeteIdentity> dict=KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( dict ); 
	if(dict.count() == 1 )
	{
		return static_cast<MSNIdentity*>(it.current())->actionMenu();
	}

	KActionMenu *m_menu=new KActionMenu(displayName(),protocolIcon(),this);

	for( ; MSNIdentity *identity=static_cast<MSNIdentity*>(it.current()); ++it )
	{
		m_menu->insert(identity->actionMenu());
	}
	return m_menu;
}

// NOTE: CALL THIS ONLY BEING CONNECTED
void MSNProtocol::slotSyncContactList()
{
/*	if ( ! mIsConnected )
	{
		return;
	}
	// First, delete D marked contacts
	QStringList localcontacts;

	contactsFile->setGroup("Default");

	contactsFile->readListEntry("Contacts",localcontacts);
	QString tmpUin;
	tmpUin.sprintf("%d",uin);
	tmp.append(tmpUin);
	cnt=contactsFile->readNumEntry("Count",0);
*/
}


/*void MSNProtocol::slotNotifySocketStatusChanged( MSNSocket::OnlineStatus status )
{
	kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: " << status <<endl;
	mIsConnected = (status == MSNSocket::Connected);
	if( status == MSNSocket::Disconnected )
	{
		KopeteMessageManagerDict sessions =
			KopeteMessageManagerFactory::factory()->protocolSessions( this );
		QIntDictIterator<KopeteMessageManager> kmmIt( sessions );
		for( ; kmmIt.current() ; ++kmmIt )
		{
			// Disconnect all active chats (but don't actually remove the
			// chat windows, the user might still want to view them!)
			MSNMessageManager *msnMM =
				dynamic_cast<MSNMessageManager *>( kmmIt.current() );
			if( msnMM )
			{
				kdDebug(14140) << "MSNProtocol::slotOnlineStatusChanged: "
					<< "Closed MSNMessageManager because the protocol socket "
					<< "closed." << endl;
				msnMM->slotCloseSession();
			}
		}

		QDictIterator<KopeteContact> it( contacts() );
		for ( ; it.current() ; ++it )
		{
			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
		}

		m_allowList.clear();
		m_blockList.clear();
		m_groupList.clear();

		mIsConnected = false;
		setStatusIcon( "msn_offline" );
//		m_openInboxAction->setEnabled(false);

		m_status = FLN;

		// Reset flags. They can't be set in the connect method, because
		// offline changes might have been made before. Instead the c'tor
		// sets the defaults, and the disconnect slot resets those defaults
		// FIXME: Can't we share this code?
		m_publicNameSyncMode = SyncFromServer;
	}
	else if( status == MSNSocket::Connecting )
	{
		for( QDictIterator<KopeteContact> it( contacts() ); it.current() ; ++it )
			static_cast<MSNContact *>( *it )->setMsnStatus( MSNProtocol::FLN );
	}
}*/


const QString MSNProtocol::protocolIcon()
{
	return "msn_online";
}

MSNProtocol::Status MSNProtocol::convertStatus( QString status )
{
	if( status == "NLN" )
		return NLN;
	else if( status == "FLN" )
		return FLN;
	else if( status == "HDN" )
		return HDN;
	else if( status == "PHN" )
		return PHN;
	else if( status == "LUN" )
		return LUN;
	else if( status == "BRB" )
		return BRB;
	else if( status == "AWY" )
		return AWY;
	else if( status == "BSY" )
		return BSY;
	else if( status == "IDL" )
		return IDL;
	else
		return FLN;
}


KActionCollection * MSNProtocol::customChatActions(KopeteMessageManager * manager)
{
	MSNMessageManager *msnMM= dynamic_cast<MSNMessageManager*>(manager);
	if(!msnMM)
		return 0L;

	return msnMM->chatActions();
}


#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

