/*
    ircguiclient.cpp

    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Kopete    (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>
    Kopete    (c) 2003-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircguiclient.h"
#include "ircaccount.h"
#include "irccontact.h"
#include "kcodecaction.h"

#include "kopetechatsession.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include <qdom.h>
#include <qptrlist.h>

IRCGUIClient::IRCGUIClient(Kopete::ChatSession *parent)
	: QObject(parent),
	  KXMLGUIClient(parent)
{
	Kopete::ContactPtrList members = parent->members();
	if( members.count() > 0 )
	{
		m_contact = static_cast<IRCContact*>( members.first() );

		/***
		FIXME: Why doesn't this work???? Have to use DOM hack below now...

		setXMLFile("ircchatui.rc");

		unplugActionList( "irccontactactionlist" );
		QPtrList<KAction> *actions = m_user->customContextMenuActions( parent );
		plugActionList( "irccontactactionlist",  *actions );
		delete actions;
		*/

		setXMLFile("ircchatui.rc");

		QDomDocument doc = domDocument();
		QDomNode menu = doc.documentElement().firstChild().firstChild();
		QPtrList<KAction> *actions = m_contact->customContextMenuActions( parent );
		if( actions )
		{
			for( KAction *a = actions->first(); a; a = actions->next() )
			{
				actionCollection()->insert( a );
				QDomElement newNode = doc.createElement( "Action" );
				newNode.setAttribute( "name", a->name() );
				menu.appendChild( newNode );
			}
		}
		else
		{
			kdDebug(14120) << k_funcinfo << "Actions == 0" << endl;
		}

		delete actions;

		setDOMDocument( doc );
	}
	else
	{
		kdDebug(14120) << k_funcinfo << "Members == 0" << endl;
	}
}

IRCGUIClient::~IRCGUIClient()
{
}

void IRCGUIClient::slotSelectCodec(QTextCodec *codec)
{
	m_contact->setCodec(codec);
}

#include "ircguiclient.moc"
