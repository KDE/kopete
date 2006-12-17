/*
  aimcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
  Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "aimcontact.h"

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include "kopeteuiglobal.h"

//liboscar
#include "oscarutils.h"
#include "contactmanager.h"

#include "aimprotocol.h"
#include "aimuserinfo.h"
#include "aimaccount.h"

AIMContact::AIMContact( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon, const OContact& ssiItem )
: AIMContactBase(account, name, parent, icon, ssiItem )
{
	mProtocol=static_cast<AIMProtocol *>(protocol());
	setOnlineStatus( AIM::Presence( AIM::Presence::Offline ).toOnlineStatus() );

	m_infoDialog = 0L;
	m_warnUserAction = 0L;

	QObject::connect( mAccount->engine(), SIGNAL( receivedUserInfo( const QString&, const UserDetails& ) ),
	                  this, SLOT( userInfoUpdated( const QString&, const UserDetails& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userIsOffline( const QString& ) ),
	                  this, SLOT( userOffline( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedProfile( const QString&, const QString& ) ),
	                  this, SLOT( updateProfile( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userWarned( const QString&, quint16, quint16 ) ),
	                  this, SLOT( gotWarning( const QString&, quint16, quint16 ) ) );
}

AIMContact::~AIMContact()
{
}

bool AIMContact::isReachable()
{
	return true;
}

QList<KAction*> *AIMContact::customContextMenuActions()
{

	QList<KAction*> *actionCollection = new QList<KAction*>();
	if ( !m_warnUserAction )
	{
		m_warnUserAction = new KAction( i18n( "&Warn User" ), 0, "warnAction" );
		QObject::connect( m_warnUserAction, SIGNAL(triggered(bool)), this, SLOT(warnUser()) );
	}
	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), 0, "actionVisibleTo");
	QObject::connect( m_actionVisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotVisibleTo()) );
	
	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), 0, "actionInvisibleTo");
	QObject::connect( m_actionInvisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotInvisibleTo()) );
	
	bool on = account()->isConnected();

	m_warnUserAction->setEnabled( on );

	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	ContactManager* ssi = account()->engine()->ssiManager();
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actionCollection->append( m_warnUserAction );

	actionCollection->append(m_actionVisibleTo);
	actionCollection->append(m_actionInvisibleTo);

	return actionCollection;
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
		setOnlineStatus( AIM::Presence( AIM::Presence::Offline ).toOnlineStatus() );
	}
}

void AIMContact::slotUserInfo()
{
	if ( !m_infoDialog)
	{
		m_infoDialog = new AIMUserInfoDialog( this, static_cast<AIMAccount*>( account() ), Kopete::UI::Global::mainWidget() );
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
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << contact << endl;

	//if they don't have an SSI alias, make sure we use the capitalization from the
	//server so their contact id looks all pretty.
	QString nickname = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( nickname.isEmpty() || Oscar::normalize( nickname ) == Oscar::normalize( contact ) )
		setNickName( contact );

	kDebug( OSCAR_AIM_DEBUG ) << k_funcinfo << "extendedStatus is " << details.extendedStatus() << endl;
	AIM::Presence presence = AIM::Presence::fromOscarStatus( details.extendedStatus(), details.userClass() );
	setOnlineStatus( presence.toOnlineStatus() );
	
	if ( presence.type() == AIM::Presence::Online )
	{
		removeProperty( mProtocol->awayMessage );
		m_haveAwayMessage = false;
	}
	else
	{
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
	if ( Oscar::normalize( userId ) == Oscar::normalize( contactId() ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Getting more contact info" << endl;
		setOnlineStatus( AIM::Presence( AIM::Presence::Online ).toOnlineStatus() );
	}
}

void AIMContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) == Oscar::normalize( contactId() ) )
	{
		kDebug(OSCAR_AIM_DEBUG) << "Setting " << userId << " offline" << endl;
		setOnlineStatus( AIM::Presence( AIM::Presence::Offline ).toOnlineStatus() );
		removeProperty( mProtocol->awayMessage );
	}
}

void AIMContact::updateProfile( const QString& contact, const QString& profile )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	setProperty( mProtocol->clientProfile, profile );
	emit updatedProfile();
}

void AIMContact::gotWarning( const QString& contact, quint16 increase, quint16 newLevel )
{
	//somebody just got bitchslapped! :O
	Q_UNUSED( increase );
	if ( Oscar::normalize( contact ) == Oscar::normalize( contactId() ) )
		m_warningLevel = newLevel;

	//TODO add a KNotify event after merge to HEAD
}

void AIMContact::closeUserInfoDialog()
{
	m_infoDialog->deleteLater();
	m_infoDialog = 0L;
}

void AIMContact::warnUser()
{
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	QString message = i18n( "<qt>Would you like to warn %1 anonymously or with your name?<br>" \
	                        "(Warning a user on AIM will result in a \"Warning Level\"" \
	                        " increasing for the user you warn. Once this level has reached a" \
	                        " certain point, they will not be able to sign on. Please do not abuse" \
	                        " this function, it is meant for legitimate practices.)</qt>", nick );


	int result = KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget(), message,
	                                               i18n( "Warn User %1?", nick ),
	                                               KGuiItem( i18n( "Warn Anonymously" ) ), KGuiItem( i18n( "Warn" ) ) );

	if ( result == KMessageBox::Yes )
		mAccount->engine()->sendWarning( contactId(), true);
	else if ( result == KMessageBox::No )
		mAccount->engine()->sendWarning( contactId(), false);
}

void AIMContact::slotVisibleTo()
{
	account()->engine()->setVisibleTo( contactId(), m_actionVisibleTo->isChecked() );
}

void AIMContact::slotInvisibleTo()
{
	account()->engine()->setInvisibleTo( contactId(), m_actionInvisibleTo->isChecked() );
}

#include "aimcontact.moc"
//kate: tab-width 4; indent-mode csands;
