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

#include <kglobal.h>
#include <kcharsets.h>
#include <klocale.h>
#include <qstringlist.h>
#include <kdebug.h>

#include "kopetemessagemanager.h"
#include "ircguiclient.h"
#include "ircaccount.h"
#include "irccontact.h"

KCodecAction::KCodecAction( const QString &text, const KShortcut &cut, 
		QObject *parent, const char *name ) : KSelectAction( text, "", cut, parent, name )
{
	QObject::connect( this, SIGNAL( activated( const QString & ) ), 
		this, SLOT(slotActivated( const QString & )) );
	
	setItems( KGlobal::charsets()->descriptiveEncodingNames() );
}

void KCodecAction::slotActivated( const QString &codec )
{
	KCharsets *c = KGlobal::charsets();
	emit activated( c->codecForName( c->encodingForName( codec ) ) );
}

void KCodecAction::setCodec( const QTextCodec *codec )
{
	QStringList myItems = comboItems();
	KCharsets *c = KGlobal::charsets();
	for( uint i = 0; i < myItems.count(); i++ )
	{
		if( c->codecForName( c->encodingForName( myItems[i] ) ) == codec )
			setCurrentItem( i );
	}
}

IRCGUIClient::IRCGUIClient( KopeteMessageManager *parent ) : QObject(parent) , KXMLGUIClient(parent)
{
	KopeteContactPtrList members = parent->members();
	m_user = static_cast<IRCContact*>( members.first() );
	
	KCodecAction *c = new KCodecAction( i18n("Select Charset"), 0, actionCollection(), "selectcharset" );
	connect( c, SIGNAL( activated( const QTextCodec * ) ), this, SLOT( slotSelectCodec( const QTextCodec *) ) );
	c->setCodec( m_user->codec() );
	
	setXMLFile("ircchatui.rc");
}

IRCGUIClient::~IRCGUIClient()
{
}

void IRCGUIClient::slotSelectCodec( const QTextCodec *codec )
{
	m_user->setCodec( codec );
}

#include "ircguiclient.moc"
