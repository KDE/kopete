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

#include "kopete.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"



/** KopeteGroupList **/
KopeteGroupList::KopeteGroupList(const QString& text, const QString& pix, const KShortcut& cut, const QObject* receiver, const char* slot, QObject* parent, const char* name)
                : KListAction(text, pix, cut, parent, name)
{
	connect( this, SIGNAL( activated() ), receiver, slot );

	connect(kopeteapp->contactList(), SIGNAL(groupAdded(const QString &)), this, SLOT(slotUpdateList()));
	connect(kopeteapp->contactList(), SIGNAL(groupRemoved(const QString &)), this, SLOT(slotUpdateList()));
	slotUpdateList();
}

KopeteGroupList::~KopeteGroupList()
{
}

void KopeteGroupList::slotUpdateList()
{
	setItems( KopeteContactList::contactList()->groups() );
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
	return new KAction( i18n("Cha&nge MetaContact"), "move", 0, recvr, slot, parent,
		name );
}

KListAction *KopeteStdAction::moveContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KopeteGroupList( i18n("&Move Contact"), "editcut", 0, recvr, slot,
		parent, name );
}

KListAction *KopeteStdAction::copyContact( const QObject *recvr,
	const char *slot, QObject* parent, const char *name )
{
	return new KopeteGroupList( i18n("Cop&y Contact"), "editcopy", 0, recvr, slot,
		parent, name );
}

KAction* KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("&Delete Contact..."), "edittrash", 0, recvr, slot,
		parent, name );
}

KListAction *KopeteStdAction::addContact(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
   KListAction *a=new KListAction(  i18n("&Add Contact"), "bookmark_add", 0, recvr, slot, parent, name );
   QStringList protocolList;

	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>( tmpprot );
		if (prot)
		{
			protocolList.append((*i).name);
		}
	}

	a->setItems( protocolList );
	return a;
}

KAction* KopeteStdAction::changeAlias(const QObject *recvr, const char *slot, QObject* parent, const char *name)
{
	return new KAction( i18n("Change A&lias"), "signature", 0, recvr, slot, parent, name );
}


#include "kopetestdaction.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

