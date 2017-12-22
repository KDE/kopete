/*
    ircguiclient.cpp

    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Kopete    (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "kopetechatsession.h"

#include <QAction>
#include <kdebug.h>
#include <klocale.h>

#include <qdom.h>
#include <QList>

IRCGUIClient::IRCGUIClient(Kopete::ChatSession *parent)
	: QObject(parent)
	, KXMLGUIClient(parent)
//	, m_contact(static_cast<IRCContact*>(parent->myself()))
{
	Q_ASSERT(m_contact);

#ifdef __GNUC__
	#warning FIXME: Why does not this work???? Have to use DOM hack below now...
#endif
/*
	setXMLFile("ircchatui.rc");

	// setup();
	unplugActionList( "irccontactactionlist" );
	plugActionList( "irccontactactionlist", m_contact->customContextMenuActions( parent ) );
*/
/*
	setXMLFile("ircchatui.rc");

	QDomDocument doc = domDocument();
	QDomNode menu = doc.documentElement().firstChild().firstChild();
	QList<QAction *> *actions = m_contact->customContextMenuActions( parent );
	for( QAction *actions, m_contact->customContextMenuActions( parent ))
	{
		actionCollection()->insert(action);
		QDomElement newNode = doc.createElement( "Action" );
		newNode.setAttribute( "name", action->name() );
		menu.appendChild( newNode );
	}
	setDOMDocument( doc );
*/
}

IRCGUIClient::~IRCGUIClient()
{
}
/*
void IRCGUIClient::updateMenu()
{
	unplugActionList( "irccontactactionlist" );

	plugActionList( "irccontactactionlist", m_contact->customContextMenuActions( parent ) );
}
*/
void IRCGUIClient::slotSelectCodec(QTextCodec *codec)
{
//	m_contact->setCodec(codec);
}

