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

#include "autoreplaceplugin.h"

#include <kgenericfactory.h>

#include <kopetecontact.h>

#include "kopetechatsessionmanager.h"
#include "kopetesimplemessagehandler.h"

#include "autoreplaceconfig.h"

K_PLUGIN_FACTORY(AutoReplacePluginFactory, registerPlugin<AutoReplacePlugin>();)
K_EXPORT_PLUGIN(AutoReplacePluginFactory( "kopete_autoreplace" ))


AutoReplacePlugin * AutoReplacePlugin::pluginStatic_ = 0L;

AutoReplacePlugin::AutoReplacePlugin( QObject *parent, const QVariantList & )
: Kopete::Plugin( AutoReplacePluginFactory::componentData(), parent )
{
	if( !pluginStatic_ )
		pluginStatic_ = this;

	m_prefs = new AutoReplaceConfig;

	// intercept inbound messages
	mInboundHandler = new Kopete::SimpleMessageHandlerFactory ( Kopete::Message::Inbound,
	        Kopete::MessageHandlerFactory::InStageToDesired, this, SLOT (slotInterceptMessage(Kopete::Message&)) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToSend(Kopete::Message&)),
		this, SLOT(slotInterceptMessage(Kopete::Message&)) );

	connect( this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()) );
}

AutoReplacePlugin::~AutoReplacePlugin()
{
	pluginStatic_ = 0L;

	delete mInboundHandler;
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

void AutoReplacePlugin::slotInterceptMessage( Kopete::Message &msg )
{
	if ( ( msg.direction() == Kopete::Message::Outbound && m_prefs->autoReplaceOutgoing() ) ||
		( msg.direction() == Kopete::Message::Inbound && m_prefs->autoReplaceIncoming() ) )
	{
		QString replaced_message = msg.plainBody();
		AutoReplaceConfig::WordsToReplace map = m_prefs->map();

		// replaces all matched words --> try to find a more 'economic' way
		// "\\b(%1)\\b" doesn't work when substituting /me.
		const QString match = "(^|\\s|\\.|\\;|\\,|\\:)(%1)(\\b)";
		AutoReplaceConfig::WordsToReplace::Iterator it;
		bool isReplaced=false;
		for ( it = map.begin(); it != map.end(); ++it )
		{
			QRegExp re( match.arg( QRegExp::escape( it.key() ) ) );
			if( re.indexIn( replaced_message ) != -1 )
			{
				QString before = re.cap(1);
				QString after = re.cap(3);
				replaced_message.replace( re, before + map.find( it.key() ).value() + after );
				isReplaced=true;
			}
		}

		if ( m_prefs->dotEndSentence() )
		{
			// eventually add . at the end of the lines, sent lines only
			replaced_message.replace( QRegExp( "([a-z])$" ), "\\1." );
			// replaced_message.replace(QRegExp( "([\\w])$" ), "\\1." );
			isReplaced=true;
		}

		if( m_prefs->capitalizeBeginningSentence() )
		{
			// eventually start each sent line with capital letter
			// TODO 	". "	 "? "	 "! " 
			replaced_message[ 0 ] = replaced_message.at( 0 ).toUpper();
			isReplaced=true;
		}

		// the message is now the one with replaced words
		if(isReplaced)
			msg.setPlainBody( replaced_message );
	}
}

#include "autoreplaceplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

