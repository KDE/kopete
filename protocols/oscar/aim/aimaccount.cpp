/*
  aimaccount.cpp  -  Oscar Protocol Plugin, AIM part

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopeteawayaction.h"
#include "kopetepassword.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimcontact.h"
#include "aimuserinfo.h"
#include "oscarmyselfcontact.h"

#include "client.h"


const DWORD AIM_ONLINE = 0x0;
const DWORD AIM_AWAY = 0x1;

namespace Kopete { class MetaContact; }

class AIMMyselfContact : public OscarMyselfContact
{
public:
	AIMMyselfContact( AIMAccount *acct ) : OscarMyselfContact( acct ) {}
	void userInfoUpdated()
	{
		if ( ( details().userClass() & 32 ) == 0 )
			setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOnline ); //we're online
		else
			setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusAway ); //we're away
	}
};


AIMAccount::AIMAccount(Kopete::Protocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, false)
{
	kdDebug(14152) << k_funcinfo << accountID << ": Called."<< endl;
	setMyself( new AIMMyselfContact( this ) );
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( parent )->statusOffline );
	QString profile = configGroup()->readEntry( "Profile",
		i18n( "Visit the Kopete website at <a href=\"http://kopete.kde.org\">http://kopete.kde.org</a>") );
}

AIMAccount::~AIMAccount()
{
}

OscarContact *AIMAccount::createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const SSI& ssiItem )
{
	AIMContact* contact = new AIMContact( this, contactId, parentContact, QString::null, ssiItem );
	if ( !ssiItem.alias().isEmpty() )
		contact->setProperty( Kopete::Global::Properties::self()->nickName(), ssiItem.alias() );

	return contact;
}

QString AIMAccount::sanitizedMessage( const Oscar::Message& message )
{
	return message.text();
}

KActionMenu* AIMAccount::actionMenu()
{
//	kdDebug(14152) << k_funcinfo << accountId() << ": Called." << endl;
	// mActionMenu is managed by Kopete::Account.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.
	KActionMenu *mActionMenu = new KActionMenu(accountId(),
		myself()->onlineStatus().iconFor( this ), this, "AIMAccount::mActionMenu");

	AIMProtocol *p = AIMProtocol::protocol();

	QString accountNick = myself()->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	mActionMenu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ),
			i18n( "%2 <%1>" ).arg( accountId(), accountNick ));

	mActionMenu->insert( new KAction( i18n("Online"), p->statusOnline.iconFor( this ), 0, this,
	             SLOT( slotGoOnline() ), mActionMenu, "AIMAccount::mActionOnline") );

	KAction* mActionAway = new Kopete::AwayAction(i18n("Away"), p->statusAway.iconFor( this ), 0, this,
	             SLOT(slotGoAway( const QString & )), this, "AIMAccount::mActionNA" );
	mActionAway->setEnabled( isConnected() );
	mActionMenu->insert( mActionAway );
	
	KAction* mActionOffline = new KAction( i18n("Offline"), p->statusOffline.iconFor(this), 0, this,
	             SLOT( slotGoOffline() ), mActionMenu, "AIMAccount::mActionOffline");
	mActionOffline->setEnabled( isConnected() );

	mActionMenu->insert( mActionOffline );
	mActionMenu->popupMenu()->insertSeparator();

	mActionMenu->insert( KopeteStdAction::contactInfo( this, SLOT( slotEditInfo() ), mActionMenu, "AIMAccount::mActionEditInfo" ) );

	return mActionMenu;
}

void AIMAccount::setAway(bool away, const QString &awayReason)
{
// 	kdDebug(14152) << k_funcinfo << accountId() << "reason is " << awayReason << endl;
	if ( away )
		engine()->setStatus( Client::Away, awayReason );
	else
		engine()->setStatus( Client::Online );
}

void AIMAccount::setUserProfile(const QString &profile)
{
	kdDebug(14152) << k_funcinfo << "called." << endl;
	static_cast<AIMContact *>(myself())->setOwnProfile(profile);
	configGroup()->writeEntry( QString::fromLatin1( "Profile" ), profile );
}

void AIMAccount::slotEditInfo()
{
	AIMUserInfoDialog *myInfo = new AIMUserInfoDialog(static_cast<AIMContact *>( myself() ), this, true, 0L, "myInfo");
	myInfo->exec(); // This is a modal dialog
}


void AIMAccount::slotGoOnline()
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
	{
		kdDebug(14152) << k_funcinfo << accountId() << " was away. welcome back." << endl;
		engine()->setStatus( Client::Online );
	}
	else if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
	{
		kdDebug(14152) << k_funcinfo << accountId() << " was offline. time to connect" << endl;
		OscarAccount::connect();
	}
	else
	{
		kdDebug(14152) << k_funcinfo << accountId() << " is already online, doing nothing" << endl;
	}
}

void AIMAccount::slotGoAway(const QString &message)
{
	kdDebug(14152) << k_funcinfo << message << endl;
	setAway(true, message);
}

void AIMAccount::disconnected( DisconnectReason reason )
{
	kdDebug( OSCAR_AIM_DEBUG ) << k_funcinfo << "Attempting to set status offline" << endl;
	myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );
	OscarAccount::disconnected( reason );
}

void AIMAccount::connectWithPassword( const QString & )
{
	kdDebug(14152) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
	
	// Get the screen name for this account
	QString screenName = accountId();
	QString server = configGroup()->readEntry( "Server", QString::fromLatin1( "login.oscar.aol.com" ) );
	uint port = configGroup()->readNumEntry( "Port", 5190 );

	Connection* c = setupConnection( server, port );
	
	QString _password = password().cachedValue();
	if ( _password.isEmpty() )
	{
		kdDebug(14150) << "Kopete is unable to attempt to sign-on to the "
			<< "AIM network because no password was specified in the "
			<< "preferences.";
	}
	else if ( myself()->onlineStatus() == static_cast<AIMProtocol*>( protocol() )->statusOffline )
	{
		kdDebug(14152) << k_funcinfo << "Logging in as " << accountId() << endl ;
		engine()->start( server, port, accountId(), _password );
		engine()->connectToServer( c, server, true /* doAuth */ );
		myself()->setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusConnecting );
	}
}

#include "aimaccount.moc"
//kate: tab-width 4; indent-mode csands;
