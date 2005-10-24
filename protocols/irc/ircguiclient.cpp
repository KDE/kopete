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
#include <QList>

IRCGUIClient::IRCGUIClient(Kopete::ChatSession *parent)
	: QObject(parent)
	, KXMLGUIClient(parent)
//	, m_contact(static_cast<IRCContact*>(parent->myself()))
{
	Q_ASSERT(m_contact);

	#warning FIXME: Why doesn't this work???? Have to use DOM hack below now...
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
	QList<KAction *> *actions = m_contact->customContextMenuActions( parent );
	for( KAction *actions, m_contact->customContextMenuActions( parent ))
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

#include "ircguiclient.moc"

