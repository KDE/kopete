/*
    ircguiclient.cpp

    Copyright (c) 2003 by Jason Keirstead        <jason@keirstead.org>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <klocale.h>
#include <kactioncollection.h>
#include <qptrlist.h>
#include <kdebug.h>
#include <qdom.h>

#include "kopetemessagemanager.h"
#include "kcodecaction.h"
#include "ircguiclient.h"
#include "ircaccount.h"
#include "irccontact.h"

IRCGUIClient::IRCGUIClient( KopeteMessageManager *parent ) : QObject(parent) , KXMLGUIClient(parent)
{
	KopeteContactPtrList members = parent->members();
	m_user = static_cast<IRCContact*>( members.first() );
	
	/***
	FIXME: Why doesn't this work???? Have to use DOM hack below now...
	
	QPtrList<KAction> actionList;
	KActionCollection *col = m_user->customContextMenuActions();

	for( int i = col->count() - 1; i >= 0; i-- )
	{
		KAction *a = col->take( col->action(i) );
		actionCollection()->insert( a );
		actionList.append( a );
	}

	setXMLFile("ircchatui.rc");
	
	kdDebug(14120) << actionList.count() << " actions" <<endl;
	unplugActionList( "irccontactactionlist" );
	plugActionList( "irccontactactionlist",  actionList );
	
	*/
	
	setXMLFile("ircchatui.rc");
	
	QDomDocument doc = domDocument();
	QDomNode menu = doc.documentElement().firstChild().firstChild();
	KActionCollection *col = m_user->customContextMenuActions();
	for( int i = col->count() - 1; i >= 0; i-- )
	{
		KAction *a = col->take( col->action(i) );
		actionCollection()->insert( a );
		QDomElement newNode = doc.createElement( "Action" );
		newNode.setAttribute( "name", a->name() );
		menu.appendChild( newNode );
	}
	
	setDOMDocument( doc );
}

IRCGUIClient::~IRCGUIClient()
{
}

void IRCGUIClient::slotSelectCodec( const QTextCodec *codec )
{
	m_user->setCodec( codec );
}

#include "ircguiclient.moc"
