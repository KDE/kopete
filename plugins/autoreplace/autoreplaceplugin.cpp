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

#include <qstylesheet.h>
#include <qmap.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
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
		SIGNAL( aboutToDisplay( KopeteMessage & ) ),
		// aboutToDisplay or aboutToReceive ?
		SLOT( slotAutoReplaceIncomingMessage( KopeteMessage & ) ) );

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
	if(msg.direction() != KopeteMessage::Outbound)
		return;
	autoReplaceMessage( msg );
}

void AutoReplacePlugin::slotAutoReplaceIncomingMessage( KopeteMessage & msg )
{
	if(msg.direction() != KopeteMessage::Inbound || !m_prefs->getAutoreplaceIncoming())
		return;
	autoReplaceMessage( msg );
}

void AutoReplacePlugin::autoReplaceMessage( KopeteMessage & msg )
{
	QString original_message = msg.plainBody();
	QString replaced_message = original_message;
	AutoReplacePreferences::WordsToReplace map = m_prefs->getMap();

	// replaces all matched words --> try to find a better way
	QString match = "\\b(%1)\\b";
	AutoReplacePreferences::WordsToReplace::Iterator it;
		for ( it = map.begin(); it != map.end(); ++it )
			replaced_message.replace(QRegExp( 
					match.arg(QRegExp(it.key()) ) ), map.find( it.key() ).data() );

	// eventually add . at the end of the lines, sent lines only
	if( m_prefs->getAddDot() && msg.direction()!=KopeteMessage::Inbound )
		// the next one is used if use no emoticons, for it will add . after
		// lines that end up with emoticons
		// replaced_message.replace(QRegExp( "[^!?]$" ), "\\1." );
		// the next one is used if use emoticons like :d AND :D
		// replaced_message.replace(QRegExp( "([^=!?()@/OoSsPpDd])$" ), "\\1." );
		// the next one is used if use emoticons like :d and NOT :D
		// replaced_message.replace(QRegExp( "([^=!?()@/ospd])$" ), "\\1." );
		// the next one is used if use emoticons like :D and NOT :d
		replaced_message.replace(QRegExp( "([^=!?()@/OSPD])$" ), "\\1." );

	// eventually start each sent line with capital letter
	if( m_prefs->getUpper() && msg.direction()!=KopeteMessage::Inbound )
		replaced_message[0] = replaced_message.at( 0 ).upper();

	// the message is now the one with replaced words
	msg.setBody( replaced_message, KopeteMessage::PlainText );
}

#include "autoreplaceplugin.moc"
