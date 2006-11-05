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

#include <kgenericfactory.h>

#include <kopetecontact.h>

#include "kopetechatsessionmanager.h"
#include "kopetesimplemessagehandler.h"

#include "autoreplaceplugin.h"
#include "autoreplaceconfig.h"

typedef KGenericFactory<AutoReplacePlugin> AutoReplacePluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_autoreplace, AutoReplacePluginFactory( "kopete_autoreplace" )  )
AutoReplacePlugin * AutoReplacePlugin::pluginStatic_ = 0L;

AutoReplacePlugin::AutoReplacePlugin( QObject *parent, const char * name, const QStringList & )
: Kopete::Plugin( AutoReplacePluginFactory::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	m_prefs = new AutoReplaceConfig;

	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToSend( Kopete::Message & ) ),
		this, SLOT( slotAboutToSend( Kopete::Message & ) ) );

	// nb this connection causes the slot to be called on in- and outbound
	// messages which suggests something is broken in the message handler
	// system!
	m_inboundHandler = new Kopete::SimpleMessageHandlerFactory( Kopete::Message::Inbound,
		Kopete::MessageHandlerFactory::InStageToSent, this, SLOT( slotAboutToSend( Kopete::Message& ) ) );

	connect( this, SIGNAL( settingsChanged() ), this, SLOT( slotSettingsChanged() ) );
}

AutoReplacePlugin::~AutoReplacePlugin()
{
	pluginStatic_ = 0L;
	delete m_inboundHandler;
	delete m_prefs;
}

AutoReplacePlugin * AutoReplacePlugin::plugin()
{
	return pluginStatic_ ;
}

void AutoReplacePlugin::slotSettingsChanged()
{
	m_prefs->load();
}

void AutoReplacePlugin::slotAboutToSend( Kopete::Message &msg )
{
	if ( ( msg.direction() == Kopete::Message::Outbound && m_prefs->autoReplaceOutgoing() ) ||
		( msg.direction() == Kopete::Message::Inbound && m_prefs->autoReplaceIncoming() ) )
	{
		QString replaced_message = msg.plainBody();
		AutoReplaceConfig::WordsToReplace map = m_prefs->map();

		// replaces all matched words --> try to find a more 'economic' way
		// "\\b(%1)\\b" doesn't work when substituting /me.
		QString match = "(^|\\s|\\.|\\;|\\,|\\:)(%1)(\\b)";
		AutoReplaceConfig::WordsToReplace::Iterator it;
		bool isReplaced=false;
		for ( it = map.begin(); it != map.end(); ++it )
		{
			QRegExp re( match.arg( QRegExp::escape( it.key() ) ) );
			if( re.search( replaced_message ) != -1 )
			{
				QString before = re.cap(1);
				QString after = re.cap(3);
				replaced_message.replace( re, before + map.find( it.key() ).data() + after );
				isReplaced=true;
			}
		}

		// the message is now the one with replaced words
		if(isReplaced)
			msg.setBody( replaced_message, Kopete::Message::PlainText );

		if( msg.direction() == Kopete::Message::Outbound )
		{
			if ( m_prefs->dotEndSentence() )
			{
				QString replaced_message = msg.plainBody();
				// eventually add . at the end of the lines, sent lines only
				replaced_message.replace( QRegExp( "([a-z])$" ), "\\1." );
				// replaced_message.replace(QRegExp( "([\\w])$" ), "\\1." );

				// the message is now the one with replaced words
				msg.setBody( replaced_message, Kopete::Message::PlainText );
			}

			if( m_prefs->capitalizeBeginningSentence() )
			{
				QString replaced_message = msg.plainBody();
				// eventually start each sent line with capital letter
				// TODO 	". "	 "? "	 "! "
				replaced_message[ 0 ] = replaced_message.at( 0 ).upper();

				// the message is now the one with replaced words
				msg.setBody( replaced_message, Kopete::Message::PlainText );
			}
		}
	}
}

#include "autoreplaceplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

