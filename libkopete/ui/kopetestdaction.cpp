/*
    kopetestdaction.cpp  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming. <bodnar42@phalynx.dhs.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>

#include "kopetegroup.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"

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
	m_groupList = QStringList();
	m_groupList << "Top Level";
	m_groupList += KopeteContactList::contactList()->groups().toStringList();
	setItems( m_groupList );
}

KAction* KopeteStdAction::chat( const QObject *recvr, const char *slot,
	QObject* parent, const char *name )
{
	return new KAction( i18n("Start &Chat..."), "mail_generic", 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::sendMessage(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("&Send Message..."), "mail_generic", 0, recvr, slot,
		parent, name );
}

KAction* KopeteStdAction::contactInfo(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("User &Info..."), "identity", 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::sendFile(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("Send &File..."), "launch", 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::viewHistory(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("View &History..."), "history", 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::addGroup(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("&Add Group..."), "folder", 0, recvr, slot, parent,
		name );
}

KAction* KopeteStdAction::changeMetaContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("Cha&nge MetaContact..."), "move", 0, recvr, slot, parent,
		name );
}

KListAction *KopeteStdAction::moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KopeteGroupListAction( i18n("&Move Contact"), "editcut", 0, recvr, slot,
		parent, name );
}

KListAction *KopeteStdAction::copyContact( const QObject *recvr,
	const char *slot, QObject* parent, const char *name )
{
	return new KopeteGroupListAction( i18n("Cop&y Contact"), "editcopy", 0, recvr, slot,
		parent, name );
}

KAction* KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("&Delete Contact"), "edittrash", 0, recvr, slot,
		parent, name );
}

KListAction *KopeteStdAction::addContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	KListAction *a=new KListAction(  i18n("&Add Contact"), "bookmark_add", 0, recvr, slot, parent, name );
	QStringList protocolList;

	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
		if( proto )
			protocolList.append( proto->displayName() );
	}

	a->setItems( protocolList );
	return a;
}

KAction* KopeteStdAction::changeAlias(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("Change A&lias..."), "signature", 0, recvr, slot, parent, name );
}

#include "kopetestdaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

