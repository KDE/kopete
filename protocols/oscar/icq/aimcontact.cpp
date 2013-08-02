/*
  aimcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
  Copyright (c) 2006,2007 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <KActionCollection>
#include <klocale.h>
#include <ktoggleaction.h>
#include <kicon.h>

//liboscar
#include "oscarutils.h"
#include "contactmanager.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "oscarstatusmanager.h"

AIMContact::AIMContact( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon )
: AIMContactBase(account, name, parent, icon )
{
	mProtocol=static_cast<ICQProtocol *>(protocol());
	setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::AIM ) );

	QObject::connect( mAccount->engine(), SIGNAL(receivedUserInfo(QString,UserDetails)),
	                  this, SLOT(userInfoUpdated(QString,UserDetails)) );
	QObject::connect( mAccount->engine(), SIGNAL(userIsOffline(QString)),
	                  this, SLOT(userOffline(QString)) );
}

AIMContact::~AIMContact()
{
}

bool AIMContact::isReachable()
{
	return account()->isConnected();
}

QList<KAction*> *AIMContact::customContextMenuActions()
{
	QList<KAction*> *actionCollection = new QList<KAction*>();

	m_actionIgnore = new KToggleAction(i18n("&Ignore"), this );
        //, "actionIgnore");
	QObject::connect( m_actionIgnore, SIGNAL(triggered(bool)), this, SLOT(slotIgnore()) );

	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), this );
        //, "actionVisibleTo");
	QObject::connect( m_actionVisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotVisibleTo()) );

	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), this );
        //, "actionInvisibleTo");
	QObject::connect( m_actionInvisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotInvisibleTo()) );

	m_selectEncoding = new KAction( i18n( "Select Encoding..." ), this );
        //, "changeEncoding" );
	m_selectEncoding->setIcon( KIcon( "character-set" ) );
	QObject::connect( m_selectEncoding, SIGNAL(triggered(bool)), this, SLOT(changeContactEncoding()) );

	bool on = account()->isConnected();
	m_actionIgnore->setEnabled(on);
	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	ContactManager* ssi = account()->engine()->ssiManager();
	m_actionIgnore->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_IGNORE ));
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actionCollection->append( m_selectEncoding );

	actionCollection->append(m_actionIgnore);
	actionCollection->append(m_actionVisibleTo);
	actionCollection->append(m_actionInvisibleTo);

	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("contactSelectEncoding"), m_selectEncoding);
	tempCollection.addAction(QLatin1String("contactIgnore"), m_actionIgnore);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysVisibleTo"), m_actionVisibleTo);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysInvisibleTo"), m_actionInvisibleTo);
	return actionCollection;
}

void AIMContact::setSSIItem( const OContact& ssiItem )
{
	if ( ssiItem.type() != 0xFFFF && ssiItem.waitingAuth() == false &&
	     onlineStatus().status() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::AIM ) );
	}

	AIMContactBase::setSSIItem( ssiItem );
}

void AIMContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	kDebug(OSCAR_ICQ_DEBUG) << contact;

	setNickName( contact );

	kDebug( OSCAR_ICQ_DEBUG ) << "extendedStatus is " << details.extendedStatus();
	Oscar::Presence presence = mProtocol->statusManager()->presenceOf( details.extendedStatus(), details.userClass() );
	setPresenceTarget( presence );

	m_mobile = ( presence.flags() & Oscar::Presence::Wireless );

	setAwayMessage( details.personalMessage() );
	if ( presence.type() != Oscar::Presence::Online && m_details.awaySinceTime() < details.awaySinceTime() ) //prevent cyclic away message requests
	{
		mAccount->engine()->requestAIMAwayMessage( contactId() );
	}

	OscarContact::userInfoUpdated( contact, details );
}

void AIMContact::userOnline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	kDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " online";
	setPresenceTarget( Oscar::Presence( Oscar::Presence::Online, Oscar::Presence::AIM ) );
}

void AIMContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	m_details.clear();

	kDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " offline";
	setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline, Oscar::Presence::AIM ) );
}

void AIMContact::slotVisibleTo()
{
	account()->engine()->setVisibleTo( contactId(), m_actionVisibleTo->isChecked() );
}

void AIMContact::slotIgnore()
{
	account()->engine()->setIgnore( contactId(), m_actionIgnore->isChecked() );
}

void AIMContact::slotInvisibleTo()
{
	account()->engine()->setInvisibleTo( contactId(), m_actionInvisibleTo->isChecked() );
}

#include "aimcontact.moc"
//kate: tab-width 4; indent-mode csands;
