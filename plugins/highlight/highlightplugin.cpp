/***************************************************************************
                          highlightplugin.cpp  -  description
                             -------------------
    begin                : mar 14 2003
    copyright            : (C) 2003 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qregexp.h>
#include <kgenericfactory.h>
#include <knotifyclient.h>

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"

#include "filter.h"
#include "highlightplugin.h"
#include "highlightconfig.h"

typedef KGenericFactory<HighlightPlugin> HighlightPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_highlight, HighlightPluginFactory( "kopete_highlight" )  )

HighlightPlugin::HighlightPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: Kopete::Plugin( HighlightPluginFactory::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToDisplay( Kopete::Message & ) ), SLOT( slotIncomingMessage( Kopete::Message & ) ) );
	connect ( this , SIGNAL( settingsChanged() ) , this , SLOT( slotSettingsChanged() ) );

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

	QPtrList<Filter> filters=m_config->filters();
	QPtrListIterator<Filter> it( filters );
	Filter *f;
	while ((f = it.current()) != 0 )
	{
		++it;
		if(f->isRegExp ?
			msg.plainBody().contains(QRegExp(f->search , f->caseSensitive)) :
			msg.plainBody().contains(f->search , f->caseSensitive) )
		{
			if(f->setBG)
				msg.setBg(f->BG);
			if(f->setFG)
				msg.setFg(f->FG);
			if(f->setImportance)
				msg.setImportance((Kopete::Message::MessageImportance)f->importance);
			if(f->playSound)
				KNotifyClient::userEvent (QString::null, KNotifyClient::Sound, KNotifyClient::Default, f->soundFN );

                        if (f->raiseView &&
                            msg.manager() && msg.manager()->view()) {
                            KopeteView *theview = msg.manager()->view();
                            theview->raise();
                        }
                        
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

