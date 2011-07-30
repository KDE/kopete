/*
  icqontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003      by Stefan Gehn <metz@gehn.net>
  Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>
  Copyright (c) 2006,2007 by Roman Jarosz <kedgedev@centrum.cz>

  Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "icqcontact.h"

#include <qtimer.h>

#include <KActionCollection>
#include <klocale.h>
#include <krandom.h>
#include <ktoggleaction.h>

#include "kopetemetacontact.h"

#include "aimprotocol.h"
#include "aimaccount.h"

#include "oscarutils.h"
#include "contactmanager.h"
#include "oscarstatusmanager.h"

ICQContact::ICQContact( Kopete::Account* account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon )
: ICQContactBase( account, name, parent, icon )
{
	mProtocol = static_cast<AIMProtocol *>(protocol());

	setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::ICQ ) );

	QObject::connect( mAccount->engine(), SIGNAL(loggedIn()), this, SLOT(loggedIn()) );
	//QObject::connect( mAccount->engine(), SIGNAL(userIsOnline(QString)), this, SLOT(userOnline(QString,UserDetails)) );
	QObject::connect( mAccount->engine(), SIGNAL(userIsOffline(QString)), this, SLOT(userOffline(QString)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedUserInfo(QString,UserDetails)),
	                  this, SLOT(userInfoUpdated(QString,UserDetails)) );
}

ICQContact::~ICQContact()
{
}

void ICQContact::setSSIItem( const OContact& ssiItem )
{
	if ( ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	if ( ssiItem.type() != 0xFFFF && ssiItem.waitingAuth() == false &&
	     onlineStatus().status() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::ICQ ) );
	}

	ICQContactBase::setSSIItem( ssiItem );
}


void ICQContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	//kDebug(OSCAR_AIM_DEBUG) << contact << contactId();
	if ( Oscar::normalize( contact  ) != Oscar::normalize( contactId() ) )
		return;

	// invalidate old away message if user was offline
	if ( !isOnline() )
		removeProperty( mProtocol->statusMessage );

	kDebug( OSCAR_AIM_DEBUG ) << "extendedStatus is " << details.extendedStatus();
	Oscar::Presence presence = mProtocol->statusManager()->presenceOf( details.extendedStatus(), details.userClass() );
	setPresenceTarget( presence );

	setAwayMessage( details.personalMessage() );
	if ( presence.type() != Oscar::Presence::Online && m_details.awaySinceTime() < details.awaySinceTime() ) //prevent cyclic away message requests
	{
		mAccount->engine()->requestAIMAwayMessage( contactId() );
	}

//TODO: don't know if we need this in aim
// 	if ( details.dcOutsideSpecified() )
// 	{
// 		if ( details.dcExternalIp().isUnspecified() )
// 			removeProperty( mProtocol->ipAddress );
// 		else
// 			setProperty( mProtocol->ipAddress, details.dcExternalIp().toString() );
// 	}

	if ( details.capabilitiesSpecified() )
		setProperty( mProtocol->clientFeatures, details.clientName() );

	OscarContact::userInfoUpdated( contact, details );
}

void ICQContact::userOnline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	kDebug(OSCAR_AIM_DEBUG) << "Setting " << userId << " online";
	setPresenceTarget( Oscar::Presence( Oscar::Presence::Online, Oscar::Presence::ICQ ) );
}

void ICQContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	m_details.clear();

	kDebug(OSCAR_AIM_DEBUG) << "Setting " << userId << " offline";
	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );
	else
		setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::ICQ ) );

	removeProperty( mProtocol->statusMessage );
}

void ICQContact::loggedIn()
{
	if ( metaContact()->isTemporary() )
		return;

	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );
}

bool ICQContact::isReachable()
{
	return true;
}

QList<KAction*> *ICQContact::customContextMenuActions()
{
	QList<KAction*> *actions = new QList<KAction*>();

	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), this );
        //, "actionVisibleTo");
	QObject::connect( m_actionVisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotVisibleTo()) );

	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), this );
        //, "actionInvisibleTo");
	QObject::connect( m_actionInvisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotInvisibleTo()) );

	bool on = account()->isConnected();

	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	ContactManager* ssi = account()->engine()->ssiManager();
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actions->append(m_actionVisibleTo);
	actions->append(m_actionInvisibleTo);

	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysVisibleTo"), m_actionVisibleTo);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysInvisibleTo"), m_actionInvisibleTo);

	return actions;
}


void ICQContact::slotVisibleTo()
{
	account()->engine()->setVisibleTo( contactId(), m_actionVisibleTo->isChecked() );
}

void ICQContact::slotInvisibleTo()
{
	account()->engine()->setInvisibleTo( contactId(), m_actionInvisibleTo->isChecked() );
}

#include "icqcontact.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
