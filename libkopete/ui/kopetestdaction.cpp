/*
    kopetestdaction.cpp  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming          <ryan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <klocale.h>
#include <kdebug.h>

#include "kopetestdaction.h"
#include "kopetecontactlist.h"

/** KopeteGroupList **/
KopeteGroupListAction::KopeteGroupListAction(const QString& text, const QString& pix, const KShortcut& cut, const QObject* receiver, const char* slot, QObject* parent, const char* name)
                : KListAction(text, pix, cut, parent, name)
{
	connect( this, SIGNAL( activated() ), receiver, slot );

	connect(KopeteContactList::contactList(), SIGNAL(groupAdded( KopeteGroup *)), this, SLOT(slotUpdateList()));
	connect(KopeteContactList::contactList(), SIGNAL(groupRemoved( KopeteGroup *)), this, SLOT(slotUpdateList()));
	slotUpdateList();
}

KopeteGroupListAction::~KopeteGroupListAction()
{
}

void KopeteGroupListAction::slotUpdateList()
{
	QStringList m_groupList;

	// Top Level is special, separate it from the other groups
	m_groupList << i18n( "Contacts are put in groups. Top Level holds all groups (but can contain contacts too) Similar to KMail's folders", "Top Level" );

	KopeteGroupList groups=KopeteContactList::contactList()->groups();
	// Add groups to our list
	for( KopeteGroup *it = groups.first(); it; it = groups.next() )
	{
		if(!it->displayName().isEmpty())	//do not add top-level groups or other
			m_groupList.append( it->displayName() );
	}

	setItems( m_groupList );
}

KAction* KopeteStdAction::chat( const QObject *recvr, const char *slot, QObject* parent, const char *name )
{
	return new KAction( i18n( "Start &Chat" ), QString::fromLatin1( "mail_generic" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "&Send Message" ), QString::fromLatin1( "mail_generic" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "User &Info" ), QString::fromLatin1( "messagebox_info" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::sendFile(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "Send &File..." ), QString::fromLatin1( "launch" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("View &History" ), QString::fromLatin1( "history" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "&Create Group" ), QString::fromLatin1( "folder" ), 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::changeMetaContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "Cha&nge Meta Contact..." ), QString::fromLatin1( "move" ), 0, recvr, slot, parent, name );
}

KListAction *KopeteStdAction::moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KopeteGroupListAction( i18n( "&Move To" ), QString::fromLatin1( "editcut" ), 0, recvr, slot, parent, name );
}

KListAction *KopeteStdAction::copyContact( const QObject *recvr, const char *slot, QObject* parent, const char *name )
{
	return new KopeteGroupListAction( i18n( "Cop&y To" ), QString::fromLatin1( "editcopy" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "&Delete Contact" ), QString::fromLatin1( "edittrash" ), 0, recvr, slot, parent, name );
}

KAction* KopeteStdAction::changeAlias(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n( "Change A&lias..." ), QString::fromLatin1( "signature" ), 0, recvr, slot, parent, name );
}

#include "kopetestdaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

