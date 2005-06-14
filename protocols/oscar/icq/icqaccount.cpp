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

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopeteawayaction.h"
#include "kopetemessage.h"

#include "client.h"
#include "icquserinfo.h"
#include "oscarsettings.h"
#include "oscarutils.h"

#include "icqcontact.h"
#include "icqprotocol.h"
#include "icqaccount.h"

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
	
	QString nickName = configGroup()->readEntry("NickName", QString::null);
	mWebAware = configGroup()->readBoolEntry( "WebAware", false );
	mHideIP = configGroup()->readBoolEntry( "HideIP", true );

	
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
	
	//actionMenu->popupMenu()->insertSeparator();
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
	setPresenceTarget( ICQ::Presence( type, pres.visibility() ) );
	myself()->setProperty( Kopete::Global::Properties::self()->awayMessage(), message );
}

void ICQAccount::setPresenceTarget( const ICQ::Presence &newPres )
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
		OscarAccount::connect( newPres.toOnlineStatus() );
	}
	else
	{
		engine()->setStatus( newPres.toOscarStatus() );
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

QString ICQAccount::sanitizedMessage( const Oscar::Message& message )
{
	if ( message.type() == 1 || message.type() == 4 )
	{
		return Kopete::Message::escape( message.text() );
	}
	else 
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "ICQ type 2 messages not supported yet. Message text:" << message.text() << endl;
	
	return QString::null;
}


#include "icqaccount.moc"

//kate: tab-width 4; indent-mode csands;
