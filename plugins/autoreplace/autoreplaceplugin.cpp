/***************************************************************************
                          autoreplaceplugin.cpp  -  description
                             -------------------
    begin                : 20030425
    copyright            : (C) 2003 by Roberto Pariset
    email                : victorheremita@fastwebnet.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>


#include <kdebug.h>
#include <kgenericfactory.h>

#include "kopetemessagemanagerfactory.h"

#include "autoreplaceplugin.h"
#include "autoreplacepreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_autoreplace, KGenericFactory<AutoReplacePlugin> );
AutoReplacePlugin * AutoReplacePlugin::pluginStatic_ = 0L;

AutoReplacePlugin::AutoReplacePlugin(
		QObject *parent, const char * name, const QStringList & )
				: KopetePlugin( parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	m_prefs = new AutoReplacePreferences ( "autoreplace", this );
	map = m_prefs->getMap();	// FIXME make sure it works if map changes (seems to work)

	// autoreplace OUTGOING messages
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( aboutToSend( KopeteMessage & ) ),
		SLOT( slotAutoReplaceOutgoingMessage( KopeteMessage & ) ) );

	// autoreplace INCOMING messages
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( aboutToSend( KopeteMessage & ) ),
		SLOT( slotAutoReplaceIncomingMessage( KopeteMessage & ) ) );

	// add a dot at the end of each line
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( aboutToSend( KopeteMessage & ) ),
		SLOT( slotAddDot( KopeteMessage & ) ) );
	
	// start with capital letter
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( aboutToSend( KopeteMessage & ) ),
		SLOT( slotCapitolize( KopeteMessage & ) ) );
}

AutoReplacePlugin::~AutoReplacePlugin()
{
	pluginStatic_ = 0L;
}

AutoReplacePlugin * AutoReplacePlugin::plugin()
{
	return pluginStatic_ ;
}

void AutoReplacePlugin::slotAutoReplaceOutgoingMessage( KopeteMessage & msg )
{
	if(msg.direction() != KopeteMessage::Outbound || !m_prefs->getAutoreplaceOutgoing())
		return;
	autoReplaceMessage( msg );
}

void AutoReplacePlugin::slotAutoReplaceIncomingMessage( KopeteMessage & msg )
{
	if(msg.direction() != KopeteMessage::Inbound || !m_prefs->getAutoreplaceIncoming())
		return;
	autoReplaceMessage( msg );
}

void AutoReplacePlugin::slotAddDot( KopeteMessage & msg )
{
	if( msg.direction() != KopeteMessage::Outbound || !m_prefs->getAddDot() )
		return;
	
	QString replaced_message = msg.plainBody();
	// eventually add . at the end of the lines, sent lines only
	replaced_message.replace(QRegExp( "([a-z])$" ), "\\1." );
//	replaced_message.replace(QRegExp( "([\\w])$" ), "\\1." );
	
	// the message is now the one with replaced words
	msg.setBody( replaced_message, KopeteMessage::PlainText );
}

void AutoReplacePlugin::slotCapitolize( KopeteMessage & msg )
{
	if( msg.direction() != KopeteMessage::Outbound || !m_prefs->getUpper() )
		return;
	
	QString replaced_message = msg.plainBody();
	// eventually start each sent line with capital letter
	// TODO 	". "	 "? "	 "! " 
	replaced_message[0] = replaced_message.at( 0 ).upper();
	
	// the message is now the one with replaced words
	msg.setBody( replaced_message, KopeteMessage::PlainText );
}

void AutoReplacePlugin::autoReplaceMessage( KopeteMessage & msg )
{
	QString replaced_message = msg.plainBody();
	AutoReplacePreferences::WordsToReplace map = m_prefs->getMap();

	// replaces all matched words --> try to find a more 'economic' way
	QString match = "\\b(%1)\\b";
	AutoReplacePreferences::WordsToReplace::Iterator it;
		for ( it = map.begin(); it != map.end(); ++it )
			replaced_message.replace(QRegExp(
					match.arg(QRegExp::escape(it.key()) ) ),
					map.find( it.key() ).data() );

	// the message is now the one with replaced words
	msg.setBody( replaced_message, KopeteMessage::PlainText );
}

#include "autoreplaceplugin.moc"
