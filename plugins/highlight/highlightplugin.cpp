/*
    highlightplugin.cpp  -  description

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "highlightplugin.h"

#include <qregexp.h>

#include <kgenericfactory.h>

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"

#include "filter.h"
#include "highlightconfig.h"

K_PLUGIN_FACTORY(HighlightPluginFactory, registerPlugin<HighlightPlugin>();)
K_EXPORT_PLUGIN(HighlightPluginFactory( "kopete_highlight" ))


HighlightPlugin::HighlightPlugin( QObject *parent, const QVariantList &/*args*/ )
: Kopete::Plugin( HighlightPluginFactory::componentData(), parent )
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message&)), SLOT(slotIncomingMessage(Kopete::Message&)) );
	connect ( this , SIGNAL(settingsChanged()) , this , SLOT(slotSettingsChanged()) );

	m_config = new HighlightConfig;

	m_config->load();
}

HighlightPlugin::~HighlightPlugin()
{
	pluginStatic_ = 0L;
	delete m_config;
}

HighlightPlugin* HighlightPlugin::plugin()
{
	return pluginStatic_ ;
}

HighlightPlugin* HighlightPlugin::pluginStatic_ = 0L;


void HighlightPlugin::slotIncomingMessage( Kopete::Message& msg )
{
	if(msg.direction() != Kopete::Message::Inbound)
		return;	// FIXME: highlighted internal/actions messages are not showed correctly in the chat window (bad style)
				//  but they should maybe be highlinghted if needed

	foreach( Filter *f, m_config->filters() )
	{
		if(f->isRegExp ?
			msg.plainBody().contains(QRegExp(f->search , f->caseSensitive?Qt::CaseSensitive:Qt::CaseInsensitive )) :
			msg.plainBody().contains(f->search , f->caseSensitive?Qt::CaseSensitive:Qt::CaseInsensitive) )
		{
			if(f->setBG)
				msg.setBackgroundColor(f->BG);
			if(f->setFG)
				msg.setForegroundColor(f->FG);
			if(f->setImportance)
				msg.setImportance((Kopete::Message::MessageImportance)f->importance);
			msg.addClass( f->className()   );
			
			break; //uh?
		}
	}
}

void HighlightPlugin::slotSettingsChanged()
{
	m_config->load();
}



#include "highlightplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

