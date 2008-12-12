/*
    YahooInviteListImpl - conference invitation dialog
    
    Copyright (c) 2004 by Duncan Mac-Vicar P.    <duncan@kde.org>
    
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

#include "yahooinvitelistimpl.h"

#include <kdebug.h>

#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>

YahooInviteListImpl::YahooInviteListImpl(QWidget *parent) : KDialog(parent)
{
	setButtons( KDialog::Cancel | KDialog::User1 );
	setEscapeButton( KDialog::Cancel );
	setButtonText( KDialog::User1, i18n("Invite") );

	QWidget* w = new QWidget( this );
	m_inviteWidget = new Ui::YahooInviteListBase();
	m_inviteWidget->setupUi( w );
	setMainWidget( w );	
	QObject::connect( this, SIGNAL(user1Clicked()), this, SLOT(slotInvite()) );
	QObject::connect( m_inviteWidget->btn_Add, SIGNAL(clicked()), this, SLOT(slotAdd()) );
	QObject::connect( m_inviteWidget->btn_Remove, SIGNAL(clicked()), this, SLOT(slotRemove()) );
	QObject::connect( m_inviteWidget->btnCustomAdd, SIGNAL(clicked()), this, SLOT(slotAddCustom()) );
        connect(this,SIGNAL(cancelClicked()),this,SLOT(slotCancel()));
	m_inviteWidget->listFriends->setSelectionMode( QAbstractItemView::ExtendedSelection );
	m_inviteWidget->listInvited->setSelectionMode( QAbstractItemView::ExtendedSelection );

	show();
}

YahooInviteListImpl::~YahooInviteListImpl()
{
	delete m_inviteWidget;
}

void YahooInviteListImpl::setRoom( const QString &room )
{
	kDebug(14180) << "Setting roomname to: " << room;

	m_room = room;
}

void YahooInviteListImpl::fillFriendList( const QStringList &buddies )
{	
	kDebug(14180) << "Adding friends: " << buddies;

	m_buddyList = buddies;
	updateListBoxes();
}

void YahooInviteListImpl::updateListBoxes()
{
	kDebug(14180) ;

	m_inviteWidget->listFriends->clear();
	m_inviteWidget->listInvited->clear();
	m_inviteWidget->listFriends->insertItems( 0, m_buddyList );
	m_inviteWidget->listFriends->sortItems();
	m_inviteWidget->listInvited->insertItems( 0, m_inviteeList );
	m_inviteWidget->listInvited->sortItems();
}

void YahooInviteListImpl::addInvitees( const QStringList &invitees )
{
	kDebug(14180) << "Adding invitees: " << invitees;

	for( QStringList::const_iterator it = invitees.begin(); it != invitees.end(); ++it )
	{
		if( !m_inviteeList.contains( *it ) )
			m_inviteeList.push_back( *it );
		if( m_buddyList.contains( *it ) )
			m_buddyList.removeAll( *it );
	}

	updateListBoxes();
}

void YahooInviteListImpl::removeInvitees( const QStringList &invitees )
{
	kDebug(14180) << "Removing invitees: " << invitees;

	for( QStringList::const_iterator it = invitees.begin(); it != invitees.end(); ++it )
	{
		if( !m_buddyList.contains( *it ) )
			m_buddyList.push_back( *it );
		if( m_inviteeList.contains( *it ) )
			m_inviteeList.removeAll( *it );
	}

	updateListBoxes();
}

void YahooInviteListImpl::addParticipant( const QString &p )
{
	m_participants.push_back( p );
}

void YahooInviteListImpl::slotInvite()
{
	kDebug(14180) ;

	if( m_inviteeList.count() )
		emit readyToInvite( m_room, m_inviteeList,m_participants, m_inviteWidget->editMessage->text() );
	accept();
}


void YahooInviteListImpl::slotCancel()
{
	kDebug(14180) ;

	reject();
}


void YahooInviteListImpl::slotAddCustom()
{
	kDebug(14180) ;

	QString userId;
	userId = m_inviteWidget->editBuddyAdd->text();
	if( userId.isEmpty() )
		return;
	
	addInvitees( QStringList(userId) );
	m_inviteWidget->editBuddyAdd->clear();
}


void YahooInviteListImpl::slotRemove()
{
	kDebug(14180) ;

	QStringList buddies;
	QList<QListWidgetItem *> items = m_inviteWidget->listInvited->selectedItems();
	foreach( QListWidgetItem *item, items )
	{
		buddies.push_back( item->text() );
	}
	removeInvitees( buddies );
}


void YahooInviteListImpl::slotAdd()
{
	kDebug(14180) ;

	QStringList buddies;
	QList<QListWidgetItem *> items = m_inviteWidget->listFriends->selectedItems();
	foreach( QListWidgetItem *item, items )
        {
		buddies.push_back( item->text() );
	}
	addInvitees( buddies );
}


#include "yahooinvitelistimpl.moc"

