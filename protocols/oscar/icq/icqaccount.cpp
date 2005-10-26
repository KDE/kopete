/*
  icqaccount.cpp  -  ICQ Account Class

  Copyright (c) 2002 by Chris TenHarmsel            <tenharmsel@staticmethod.net>
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include <qfile.h>
#include <qimage.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "kopeteawayaction.h"
#include "kopetemessage.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"

#include "client.h"
#include "icquserinfo.h"
#include "oscarsettings.h"
#include "oscarutils.h"
#include "ssimanager.h"

#include "icqcontact.h"
#include "icqprotocol.h"
#include "icqaccount.h"

#include "oscarvisibilitydialog.h"

ICQMyselfContact::ICQMyselfContact( ICQAccount *acct ) : OscarMyselfContact( acct )
{
	QObject::connect( acct->engine(), SIGNAL( loggedIn() ), this, SLOT( fetchShortInfo() ) );
	QObject::connect( acct->engine(), SIGNAL( receivedIcqShortInfo( const QString& ) ),
	                  this, SLOT( receivedShortInfo( const QString& ) ) );
}

void ICQMyselfContact::userInfoUpdated()
{
	DWORD extendedStatus = details().extendedStatus();
	kdDebug( OSCAR_ICQ_DEBUG ) << k_funcinfo << "extendedStatus is " << QString::number( extendedStatus, 16 ) << endl;
	ICQ::Presence presence = ICQ::Presence::fromOscarStatus( extendedStatus & 0xffff );
	setOnlineStatus( presence.toOnlineStatus() );
}

void ICQMyselfContact::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	ICQShortInfo shortInfo = static_cast<ICQAccount*>( account() )->engine()->getShortInfo( contact );
	if ( !shortInfo.nickname.isEmpty() )
		setProperty( Kopete::Global::Properties::self()->nickName(), shortInfo.nickname );
}

void ICQMyselfContact::fetchShortInfo()
{
	static_cast<ICQAccount*>( account() )->engine()->requestShortInfo( contactId() );
}

ICQAccount::ICQAccount(Kopete::Protocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, true)
{
	kdDebug(14152) << k_funcinfo << accountID << ": Called."<< endl;
	setMyself( new ICQMyselfContact( this ) );
	myself()->setOnlineStatus( ICQ::Presence( ICQ::Presence::Offline, ICQ::Presence::Visible ).toOnlineStatus() );

	m_visibilityDialog = 0;

	QString nickName = configGroup()->readEntry("NickName", QString::null);
	mWebAware = configGroup()->readBoolEntry( "WebAware", false );
	mHideIP = configGroup()->readBoolEntry( "HideIP", true );

	QObject::connect( Kopete::ContactList::self(), SIGNAL( globalIdentityChanged( const QString&, const QVariant& ) ),
	                  this, SLOT( slotGlobalIdentityChanged( const QString&, const QVariant& ) ) );
	
	QObject::connect( engine(), SIGNAL( iconNeedsUploading() ), this, SLOT( slotSendBuddyIcon() ) );

	//setIgnoreUnknownContacts(pluginData(protocol(), "IgnoreUnknownContacts").toUInt() == 1);

	/* FIXME: need to do this when web aware or hide ip change
	if(isConnected() && (oldhideip != mHideIP || oldwebaware != mWebAware))
	{
		kdDebug(14153) << k_funcinfo <<
			"sending status to reflect HideIP and WebAware settings" << endl;
		//setStatus(mStatus, QString::null);
	}*/
}

ICQAccount::~ICQAccount()
{
}

ICQProtocol* ICQAccount::protocol()
{
	return static_cast<ICQProtocol*>(OscarAccount::protocol());
}


ICQ::Presence ICQAccount::presence()
{
	return ICQ::Presence::fromOnlineStatus( myself()->onlineStatus() );
}


KActionMenu* ICQAccount::actionMenu()
{
	KActionMenu* actionMenu = Kopete::Account::actionMenu();

	actionMenu->popupMenu()->insertSeparator();

	KToggleAction* actionInvisible =
	    new KToggleAction( i18n( "In&visible" ),
	                       ICQ::Presence( presence().type(), ICQ::Presence::Invisible ).toOnlineStatus().iconFor( this ),
	                       0, this, SLOT( slotToggleInvisible() ), this );
	actionInvisible->setChecked( presence().visibility() == ICQ::Presence::Invisible );
	actionMenu->insert( actionInvisible );

	actionMenu->popupMenu()->insertSeparator();
	actionMenu->insert( new KToggleAction( i18n( "Set Visibility..." ), 0, 0,
	                                       this, SLOT( slotSetVisiblility() ), this,
	                                       "ICQAccount::mActionSetVisibility") );
	//actionMenu->insert( new KToggleAction( i18n( "Send &SMS..." ), 0, 0, this, SLOT( slotSendSMS() ), this, "ICQAccount::mActionSendSMS") );

	return actionMenu;
}


void ICQAccount::connectWithPassword( const QString &password )
{
	if ( password.isNull() )
		return;

	kdDebug(14153) << k_funcinfo << "accountId='" << accountId() << "'" << endl;

	Kopete::OnlineStatus status = initialStatus();
	if ( status == Kopete::OnlineStatus() &&
	     status.status() == Kopete::OnlineStatus::Unknown )
		//use default online in case of invalid online status for connecting
		status = Kopete::OnlineStatus( Kopete::OnlineStatus::Online );
	ICQ::Presence pres = ICQ::Presence::fromOnlineStatus( status );
	bool accountIsOffline = ( presence().type() == ICQ::Presence::Offline ||
	                          myself()->onlineStatus() == protocol()->statusManager()->connectingStatus() );

	if ( accountIsOffline )
	{
		myself()->setOnlineStatus( protocol()->statusManager()->connectingStatus() );
		QString icqNumber = accountId();
		kdDebug(14153) << k_funcinfo << "Logging in as " << icqNumber << endl ;
		QString server = configGroup()->readEntry( "Server", QString::fromLatin1( "login.oscar.aol.com" ) );
		uint port = configGroup()->readNumEntry( "Port", 5190 );
		Connection* c = setupConnection( server, port );

		//set up the settings for the account
		Oscar::Settings* oscarSettings = engine()->clientSettings();
		oscarSettings->setWebAware( configGroup()->readBoolEntry( "WebAware", false ) );
		oscarSettings->setHideIP( configGroup()->readBoolEntry( "HideIP", true ) );
		oscarSettings->setRequireAuth( configGroup()->readBoolEntry( "RequireAuth", false ) );
		oscarSettings->setRespectRequireAuth( configGroup()->readBoolEntry( "RespectRequireAuth", true ) );
		//FIXME: also needed for the other call to setStatus (in setPresenceTarget)
		DWORD status = pres.toOscarStatus();

		if ( !mHideIP )
			status |= ICQ::StatusCode::SHOWIP;
		if ( mWebAware )
			status |= ICQ::StatusCode::WEBAWARE;

		engine()->setIsIcq( true );
		engine()->setStatus( status );
		engine()->start( server, port, accountId(), password );
		engine()->connectToServer( c, server, true /* doAuth */ );

	}
}

void ICQAccount::disconnected( DisconnectReason reason )
{
	kdDebug(14153) << k_funcinfo << "Attempting to set status offline" << endl;
	ICQ::Presence presOffline = ICQ::Presence( ICQ::Presence::Offline, presence().visibility() );
	myself()->setOnlineStatus( presOffline.toOnlineStatus() );
	OscarAccount::disconnected( reason );
}


void ICQAccount::slotToggleInvisible()
{
	using namespace ICQ;
	setInvisible( (presence().visibility() == Presence::Visible) ? Presence::Invisible : Presence::Visible );
}

void ICQAccount::slotSetVisiblility()
{
	if( !isConnected() )
	{
		KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
		                    i18n("You must be online to set users visibility."),
		                    i18n("ICQ Plugin") );
		return;
	}
	
	if ( !m_visibilityDialog )
	{
		m_visibilityDialog = new OscarVisibilityDialog( engine(), Kopete::UI::Global::mainWidget() );
		QObject::connect( m_visibilityDialog, SIGNAL( closing() ),
		                  this, SLOT( slotVisibilityDialogClosed() ) );
		
		//add all contacts;
		OscarVisibilityDialog::ContactMap contactMap;
		//temporary map for faster lookup of contactId
		QMap<QString, QString> revContactMap;
		
		QValueList<Oscar::SSI> contactList = engine()->ssiManager()->contactList();
		QValueList<Oscar::SSI>::const_iterator it, cEnd = contactList.constEnd();
		
		for ( it = contactList.constBegin(); it != cEnd; ++it )
		{
			QString contactId = ( *it ).name();
			
			OscarContact* oc = dynamic_cast<OscarContact*>( contacts()[( *it ).name()] );
			if ( oc )
			{	//for better orientation in lists use nickName and icq number
				QString screenName( "%1 (%2)" );
				screenName = screenName.arg( oc->nickName(), contactId);
				contactMap.insert( screenName, contactId );
				revContactMap.insert( contactId, screenName );
			}
			else
			{
				contactMap.insert( contactId, contactId );
				revContactMap.insert( contactId, contactId );
			}
		}
		m_visibilityDialog->addContacts( contactMap );
		
		//add contacts from visible list
		QStringList tmpList;
		
		contactList = engine()->ssiManager()->visibleList();
		cEnd = contactList.constEnd();
		
		for ( it = contactList.constBegin(); it != cEnd; ++it )
			tmpList.append( revContactMap[( *it ).name()] );
		
		m_visibilityDialog->addVisibleContacts( tmpList );
		
		//add contacts from invisible list
		tmpList.clear();
		
		contactList = engine()->ssiManager()->invisibleList();
		cEnd = contactList.constEnd();
		
		for ( it = contactList.constBegin(); it != cEnd; ++it )
			tmpList.append( revContactMap[( *it ).name()] );
		
		m_visibilityDialog->addInvisibleContacts( tmpList );
		
		m_visibilityDialog->resize( 550, 350 );
		m_visibilityDialog->show();
	}
	else
	{
		m_visibilityDialog->raise();
	}
}

void ICQAccount::slotVisibilityDialogClosed()
{
	m_visibilityDialog->delayedDestruct();
	m_visibilityDialog = 0L;
}

void ICQAccount::setAway( bool away, const QString &awayReason )
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	if ( away )
		setPresenceType( ICQ::Presence::Away, awayReason );
	else
		setPresenceType( ICQ::Presence::Online );
}


void ICQAccount::setInvisible( ICQ::Presence::Visibility vis )
{
	ICQ::Presence pres = presence();
	if ( vis == pres.visibility() )
		return;

	kdDebug(14153) << k_funcinfo << "changing invisible setting to " << (int)vis << endl;
	setPresenceTarget( ICQ::Presence( pres.type(), vis ) );
}

void ICQAccount::setPresenceType( ICQ::Presence::Type type, const QString &message )
{
	Q_UNUSED( message );
	ICQ::Presence pres = presence();
	kdDebug(14153) << k_funcinfo << "new type=" << (int)type << ", old type=" << (int)pres.type() << endl;
	//setAwayMessage(awayMessage);
	setPresenceTarget( ICQ::Presence( type, pres.visibility() ), message );
	myself()->setProperty( Kopete::Global::Properties::self()->awayMessage(), message );
}

void ICQAccount::setPresenceTarget( const ICQ::Presence &newPres, const QString &message )
{
	bool targetIsOffline = (newPres.type() == ICQ::Presence::Offline);
	bool accountIsOffline = ( presence().type() == ICQ::Presence::Offline ||
	                          myself()->onlineStatus() == protocol()->statusManager()->connectingStatus() );

	if ( targetIsOffline )
	{
		OscarAccount::disconnect();
		// allow toggling invisibility when offline
		myself()->setOnlineStatus( newPres.toOnlineStatus() );
	}
	else if ( accountIsOffline )
	{
		// set status message if given
		if ( ! message.isEmpty() )
			engine()->setStatusMessage( message );
		OscarAccount::connect( newPres.toOnlineStatus() );
	}
	else
	{
		engine()->setStatus( newPres.toOscarStatus(), message );
	}
}


void ICQAccount::setOnlineStatus( const Kopete::OnlineStatus& status, const QString& reason )
{
	if ( status.status() == Kopete::OnlineStatus::Invisible )
	{
		// called from outside, i.e. not by our custom action menu entry...

		if ( presence().type() == ICQ::Presence::Offline )
		{
			// ...when we are offline go online invisible.
			setPresenceTarget( ICQ::Presence( ICQ::Presence::Online, ICQ::Presence::Invisible ) );
		}
		else
		{
			// ...when we are not offline set invisible.
			setInvisible( ICQ::Presence::Invisible );
		}
	}
	else
	{
		setPresenceType( ICQ::Presence::fromOnlineStatus( status ).type(), reason );
	}
}


OscarContact *ICQAccount::createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem )
{
	ICQContact* contact = new ICQContact( this, contactId, parentContact, QString::null, ssiItem );
	if ( !ssiItem.alias().isEmpty() )
		contact->setProperty( Kopete::Global::Properties::self()->nickName(), ssiItem.alias() );

	if ( isConnected() )
		contact->loggedIn();

	return contact;
}

QString ICQAccount::sanitizedMessage( const QString& message )
{
	return Kopete::Message::escape( message );
}


void ICQAccount::slotGlobalIdentityChanged( const QString& key, const QVariant& value )
{
	//do something with the photo
	kdDebug(14153) << k_funcinfo << "Global identity changed" << endl;
	kdDebug(14153) << k_funcinfo << "key: " << key << endl;
	kdDebug(14153) << k_funcinfo << "value: " << value << endl;
	if ( key == Kopete::Global::Properties::self()->nickName().key() )
	{
		//edit ssi item to change alias (if possible)
	}
	
	if ( key == Kopete::Global::Properties::self()->photo().key() )
	{
		setBuddyIcon( value.toString() );
	}
}

void ICQAccount::setBuddyIcon( KURL url )
{	
	if ( url.path().isEmpty() )
	{
		myself()->removeProperty( Kopete::Global::Properties::self()->photo() );
	}
	else
	{
		QImage image( url.path() );
		if ( image.isNull() )
			return;
		
		image = image.smoothScale( 52, 64, QImage::ScaleMax );
		if(image.width() > image.height()) {
			image = image.copy((image.width()-image.height())/2, 0, image.height(), image.height());
		}
		else if(image.height() > image.width()) {
			image = image.copy(0, (image.height()-image.width())/2, image.width(), image.width());
		}
		
		QString newlocation( locateLocal( "appdata", "oscarpictures/"+ accountId() + ".jpg" ) );
		
		kdDebug(14153) << k_funcinfo << "Saving buddy icon: " << newlocation << endl;
		if ( !image.save( newlocation, "JPEG" ) )
			return;
		
		myself()->setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
	}
	
	slotBuddyIconChanged();
}

void ICQAccount::slotBuddyIconChanged()
{
	// need to disconnect because we could end up with many connections
	QObject::disconnect( engine(), SIGNAL( iconServerConnected() ), this, SLOT( slotBuddyIconChanged() ) );
	if ( !engine()->isActive() )
	{
		QObject::connect( engine(), SIGNAL( iconServerConnected() ), this, SLOT( slotBuddyIconChanged() ) );
		return;
	}
	
	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();
	
	SSIManager* ssi = engine()->ssiManager();
	Oscar::SSI item = ssi->findItemForIconByRef( 1 );
	
	if ( photoPath.isEmpty() )
	{
		if ( item )
		{
			kdDebug(14153) << k_funcinfo << "Removing icon hash item from ssi" << endl;
			Oscar::SSI s(item);
			
			//remove hash and alias
			QValueList<TLV> tList( item.tlvList() );
			TLV t = Oscar::findTLV( tList, 0x00D5 );
			if ( t )
				tList.remove( t );
			
			t = Oscar::findTLV( tList, 0x0131 );
			if ( t )
				tList.remove( t );
			
			item.setTLVList( tList );
			//s is old, item is new. modification will occur
			engine()->modifySSIItem( s, item );
		}
	}
	else
	{
		QFile iconFile( photoPath );
		iconFile.open( IO_ReadOnly );
		
		KMD5 iconHash;
		iconHash.update( iconFile );
		kdDebug(14153) << k_funcinfo  << "hash is :" << iconHash.hexDigest() << endl;
	
		//find old item, create updated item
		if ( !item )
		{
			kdDebug(14153) << k_funcinfo << "no existing icon hash item in ssi. creating new" << endl;
			
			TLV t;
			t.type = 0x00D5;
			t.data.resize( 18 );
			t.data[0] = 0x01;
			t.data[1] = 0x10;
			memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
			t.length = t.data.size();
			
			//alias, it's always empty
			TLV t2;
			t2.type = 0x0131;
			t2.length = 0;
			
			QValueList<Oscar::TLV> list;
			list.append( t );
			list.append( t2 );
			
			Oscar::SSI s( "1", 0, ssi->nextContactId(), ROSTER_BUDDYICONS, list );
			
			//item is a non-valid ssi item, so the function will add an item
			kdDebug(14153) << k_funcinfo << "setting new icon item" << endl;
			engine()->modifySSIItem( item, s );
		}
		else
		{ //found an item
			Oscar::SSI s(item);
			kdDebug(14153) << k_funcinfo << "modifying old item in ssi." << endl;
			QValueList<TLV> tList( item.tlvList() );
			
			TLV t = Oscar::findTLV( tList, 0x00D5 );
			if ( t )
				tList.remove( t );
			else
				t.type = 0x00D5;
			
			t.data.resize( 18 );
			t.data[0] = 0x01;
			t.data[1] = 0x10;
			memcpy(t.data.data() + 2, iconHash.rawDigest(), 16);
			t.length = t.data.size();
			tList.append( t );
			
			//add empty alias
			t = Oscar::findTLV( tList, 0x0131 );
			if ( !t )
			{
				t.type = 0x0131;
				t.length = 0;
				tList.append( t );
			}
			
			item.setTLVList( tList );
			//s is old, item is new. modification will occur
			engine()->modifySSIItem( s, item );
		}
		iconFile.close();
	}
}

void ICQAccount::slotSendBuddyIcon()
{
	//need to disconnect because we could end up with many connections
	QObject::disconnect( engine(), SIGNAL( iconServerConnected() ), this, SLOT( slotSendBuddyIcon() ) );
	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();
	if ( photoPath.isEmpty() )
		return;
	
	kdDebug(14153) << k_funcinfo << photoPath << endl;
	QFile iconFile( photoPath );
	
	if ( iconFile.open( IO_ReadOnly ) )
	{
		if ( !engine()->hasIconConnection() )
		{
			//will send icon when we connect to icon server
			QObject::connect( engine(), SIGNAL( iconServerConnected() ),
			                  this, SLOT( slotSendBuddyIcon() ) );
			return;
		}
		QByteArray imageData = iconFile.readAll();
		engine()->sendBuddyIcon( imageData );
	}
}


#include "icqaccount.moc"

//kate: tab-width 4; indent-mode csands;
