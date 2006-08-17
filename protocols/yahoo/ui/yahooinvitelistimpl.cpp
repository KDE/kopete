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

#include <qlistbox.h>
#include <qlineedit.h>

YahooInviteListImpl::YahooInviteListImpl(QWidget *parent, const char *name) : YahooInviteListBase(parent,name)
{
	listFriends->setSelectionMode( QListBox::Extended );
	listInvited->setSelectionMode( QListBox::Extended );
}

YahooInviteListImpl::~YahooInviteListImpl()
{
}

void YahooInviteListImpl::setRoom( const QString &room )
{
	kdDebug(14180) << k_funcinfo << "Setting roomname to: " << room << endl;

	m_room = room;
}

void YahooInviteListImpl::fillFriendList( const QStringList &buddies )
{	
	kdDebug(14180) << k_funcinfo << "Adding friends: " << buddies << endl;

	m_buddyList = buddies;
	updateListBoxes();
}

void YahooInviteListImpl::updateListBoxes()
{
	kdDebug(14180) << k_funcinfo << endl;

	listFriends->clear();
	listInvited->clear();
	listFriends->insertStringList( m_buddyList );
	listFriends->sort();
	listInvited->insertStringList( m_inviteeList );
	listInvited->sort();
}

void YahooInviteListImpl::addInvitees( const QStringList &invitees )
{
	kdDebug(14180) << k_funcinfo << "Adding invitees: " << invitees << endl;

	for( QStringList::const_iterator it = invitees.begin(); it != invitees.end(); it++ )
	{
		if( m_inviteeList.find( *it ) == m_inviteeList.end() )
			m_inviteeList.push_back( *it );
		if( m_buddyList.find( *it ) != m_buddyList.end() )
			m_buddyList.remove( *it );
	}

	updateListBoxes();
}

void YahooInviteListImpl::removeInvitees( const QStringList &invitees )
{
	kdDebug(14180) << k_funcinfo << "Removing invitees: " << invitees << endl;

	for( QStringList::const_iterator it = invitees.begin(); it != invitees.end(); it++ )
	{
		if( m_buddyList.find( *it ) == m_buddyList.end() )
			m_buddyList.push_back( *it );
		if( m_inviteeList.find( *it ) != m_inviteeList.end() )
			m_inviteeList.remove( *it );
	}

	updateListBoxes();
}

void YahooInviteListImpl::addParticipant( const QString &p )
{
	m_participants.push_back( p );
}

void YahooInviteListImpl::btnInvite_clicked()
{
	kdDebug(14180) << k_funcinfo << endl;

	if( m_inviteeList.count() )
		emit readyToInvite( m_room, m_inviteeList,m_participants, editMessage->text() );
	QDialog::accept();
}


void YahooInviteListImpl::btnCancel_clicked()
{
	kdDebug(14180) << k_funcinfo << endl;

	QDialog::reject();
}


void YahooInviteListImpl::btnAddCustom_clicked()
{
	kdDebug(14180) << k_funcinfo << endl;

	QString userId;
	userId = editBuddyAdd->text();
	if( userId.isEmpty() )
		return;
	
	addInvitees( QStringList(userId) );
	editBuddyAdd->clear();
}


void YahooInviteListImpl::btnRemove_clicked()
{
	kdDebug(14180) << k_funcinfo << endl;

	QStringList buddies;
	for( uint i=0; i<listInvited->count(); i++ )
	{
		if (listInvited->isSelected(i))
		{
			buddies.push_back( listInvited->text(i) );
		}
	}
	removeInvitees( buddies );
}


void YahooInviteListImpl::btnAdd_clicked()
{
	kdDebug(14180) << k_funcinfo << endl;

	QStringList buddies;
	for( uint i=0; i<listFriends->count(); i++ )
	{
		if (listFriends->isSelected(i))
		{
			buddies.push_back( listFriends->text(i) );
		}
	}
	addInvitees( buddies );
}


#include "yahooinvitelistimpl.moc"




