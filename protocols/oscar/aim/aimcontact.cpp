/*
  aimcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
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

#include <time.h>

#include <qapplication.h>
#include <qregexp.h>

#include <kactionclasses.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

//liboscar
#include "client.h"
#include "oscartypes.h"
#include "oscarutils.h"

#include "aimprotocol.h"
#include "aimuserinfo.h"
#include "aimcontact.h"
#include "aimaccount.h"

AIMContact::AIMContact( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon, const Oscar::SSI& ssiItem )
: OscarContact(account, name, parent, icon, ssiItem )
{
	mProtocol=static_cast<AIMProtocol *>(protocol());
	setOnlineStatus( mProtocol->statusOffline );
	
	m_infoDialog = 0L;
	m_warnUserAction = 0L;
	mUserProfile="";
	m_haveAwayMessage = false;
	
	QObject::connect( mAccount->engine(), SIGNAL( receivedUserInfo( const QString&, const UserDetails& ) ),
	                  this, SLOT( userInfoUpdated( const QString&, const UserDetails& ) ) );
	//QObject::connect( mAccount->engine(), SIGNAL( userIsOnline( const QString& ) ), this, SLOT( userOnline( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userIsOffline( const QString& ) ), this, SLOT( userOffline( const QString& ) ) );	
	QObject::connect( mAccount->engine(), SIGNAL( receivedAwayMessage( const QString&, const QString& ) ),
	                  this, SLOT( updateAwayMessage( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedProfile( const QString&, const QString& ) ),
	                  this, SLOT( updateProfile( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userWarned( const QString&, Q_UINT16, Q_UINT16 ) ),
	                  this, SLOT( gotWarning( const QString&, Q_UINT16, Q_UINT16 ) ) );
}

AIMContact::~AIMContact()
{
}

bool AIMContact::isReachable()
{
	return true;
}

QPtrList<KAction> *AIMContact::customContextMenuActions()
{
	
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();
	if ( !m_warnUserAction )
	{
		m_warnUserAction = new KAction( i18n( "&Warn User" ), 0, this, SLOT( warnUser() ), this, "warnAction" );
	}
	
	m_warnUserAction->setEnabled( account()->isConnected() );
	
	actionCollection->append( m_warnUserAction );
	return actionCollection;
}

void AIMContact::setOwnProfile(const QString &profile)
{
	kdDebug(14152) << k_funcinfo << "Called." << endl;
	if ( this == account()->myself() )
	{
		mUserProfile = profile;
	}
}

const QString AIMContact::awayMessage()
{
	return property(mProtocol->awayMessage).value().toString();
}

void AIMContact::setAwayMessage(const QString &message)
{
	kdDebug(14152) << k_funcinfo <<
		"Called for '" << contactId() << "', away msg='" << message << "'" << endl;
	QString filteredMessage = message;
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>")),
		QString::fromLatin1("\\1"));
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>")),
		QString::fromLatin1("\\1") );
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[fF][oO][nN][tT].*>(.*)</[fF][oO][nN][tT]>")),
		QString::fromLatin1("\\1") );
	setProperty(mProtocol->awayMessage, filteredMessage);
}

int AIMContact::warningLevel() const
{
	return m_warningLevel;
}

void AIMContact::updateSSIItem()
{
	if ( m_ssiItem.type() != 0xFFFF && m_ssiItem.waitingAuth() == false &&
	     onlineStatus() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );
	}
}

void AIMContact::slotUserInfo()
{
	if ( !m_infoDialog)
	{
		m_infoDialog = new AIMUserInfoDialog( this, static_cast<AIMAccount*>( account() ), false, Kopete::UI::Global::mainWidget(), 0 );
		if( !m_infoDialog )
			return;
		connect( m_infoDialog, SIGNAL( finished() ), this, SLOT( closeUserInfoDialog() ) );
		m_infoDialog->show();
		if ( mAccount->isConnected() )
		{
			mAccount->engine()->requestAIMProfile( contactId() );
			mAccount->engine()->requestAIMAwayMessage( contactId() );
		}
	}
	else
		m_infoDialog->raise();
}

void AIMContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	if ( contact.lower() != contactId().lower() )
		return;

	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << contact << endl;
	
	//if they don't have an SSI alias, make sure we use the capitalization from the
	//server so their contact id looks all pretty.
	QString nickname = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( Oscar::normalize( nickname ) == Oscar::normalize( details.userId() ) )
		setProperty( Kopete::Global::Properties::self()->nickName(), details.userId() );
	
	if ( ( details.userClass() & 32 ) == 0 )
	{
		setOnlineStatus( mProtocol->statusOnline ); //we're online
		removeProperty( mProtocol->awayMessage );
		m_haveAwayMessage = false;
	}
	else
	{
		setOnlineStatus( mProtocol->statusAway ); //we're away
		if ( !m_haveAwayMessage ) //prevent cyclic away message requests
		{
			mAccount->engine()->requestAIMAwayMessage( contactId() );
			m_haveAwayMessage = true;
		}
	}
	
	OscarContact::userInfoUpdated( contact, details );
}



void AIMContact::userOnline( const QString& userId )
{
	if ( userId.lower() == contactId().lower() )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Getting more contact info" << endl;
		setOnlineStatus( mProtocol->statusOnline );
	}
}

void AIMContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) == Oscar::normalize( contactId() ) )
	{
		setOnlineStatus( mProtocol->statusOffline );
		removeProperty( mProtocol->awayMessage );
	}
}

void AIMContact::updateAwayMessage( const QString& contact, const QString& message )
{
	if ( contact.lower() != contactId().lower() )
		return;
	else
	{
		if ( message.isEmpty() )
		{
			removeProperty( mProtocol->awayMessage );
			setOnlineStatus( mProtocol->statusOnline );
			m_haveAwayMessage = false;
		}
		else
		{
			m_haveAwayMessage = true;
			setAwayMessage( message );
			setOnlineStatus( mProtocol->statusAway );
		}
	}
	
	emit updatedProfile();
}

void AIMContact::updateProfile( const QString& contact, const QString& profile )
{
	if ( contact.lower() != contactId().lower() )
		return;
	
	setProperty( mProtocol->clientProfile, profile );
	emit updatedProfile();
}

void AIMContact::gotWarning( const QString& contact, Q_UINT16 increase, Q_UINT16 newLevel )
{
	//somebody just got bitchslapped! :O
	Q_UNUSED( increase );
	if ( Oscar::normalize( contact ) == Oscar::normalize( contactId() ) )
		m_warningLevel = newLevel;
	
	//TODO add a KNotify event after merge to HEAD
}

void AIMContact::closeUserInfoDialog()
{
	m_infoDialog->delayedDestruct();
	m_infoDialog = 0L;
}

void AIMContact::warnUser()
{
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	QString message = i18n( "<qt>Would you like to warn %1 anonymously or with your name?<br>" \
	                        "(Warning a user on AIM will result in a \"Warning Level\"" \
	                        " increasing for the user you warn. Once this level has reached a" \
	                        " certain point, they will not be able to sign on. Please do not abuse" \
	                        " this function, it is meant for legitimate practices.)</qt>" ).arg( nick );
	
	
	int result = KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget(), message,
	                                               i18n( "Warn User %1?" ).arg( nick ),
	                                               i18n( "Warn Anonymously" ), i18n( "Warn" ) );
	
	if ( result == KMessageBox::Yes )
		mAccount->engine()->sendWarning( contactId(), true);
	else if ( result == KMessageBox::No )
		mAccount->engine()->sendWarning( contactId(), false);
}

#include "aimcontact.moc"
//kate: tab-width 4; indent-mode csands;
