/***************************************************************************
                          highlightplugin.cpp  -  description
                             -------------------
    begin                : mar 14 2003
    copyright            : (C) 2003 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstylesheet.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"

#include "filter.h"
#include "highlightplugin.h"
#include "highlightpreferences.h"


K_EXPORT_COMPONENT_FACTORY( kopete_highlight, KGenericFactory<HighlightPlugin> );

HighlightPlugin::HighlightPlugin( QObject *parent, const char *name, const QStringList &/*args*/ ) :  KopetePlugin( parent, name ) 
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), SLOT( slotIncomingMessage( KopeteMessage & ) ) );
	
	//add the default filter
	Filter *filtre=newFilter();
	filtre->search="kopete";
	filtre->setFG=true;
	filtre->FG=QColor(0x0000FF);
	filtre->displayName="Kopete";
	
	//TODO: found a pixmap
	m_prefs = new HighlightPreferences ( "highlight", this );

}

HighlightPlugin::~HighlightPlugin()
{
	pluginStatic_ = 0L;
	for(Filter *f=m_filters.first() ; f; f=m_filters.next() )
	{
		delete f;
	}
}

HighlightPlugin* HighlightPlugin::plugin()
{
	return pluginStatic_ ;
}

HighlightPlugin* HighlightPlugin::pluginStatic_ = 0L;



/*KActionCollection *HighlightPlugin::customContextMenuActions(KopeteMetaContact *m)
{
	delete m_collection;

	m_collection = new KActionCollection(this);

	KAction *action=new KAction( i18n("&Select Highlight Public Key"), "kgpg", 0, this, SLOT (slotSelectContactKey()), m_collection);

	m_collection->insert(action);
	m_currentMetaContact=m;
	return m_collection;
}*/

void HighlightPlugin::slotIncomingMessage( KopeteMessage& msg )
{
	for(Filter *f=m_filters.first() ; f; f=m_filters.next() )
	{
		if(f->isRegExp ?
			msg.plainBody().contains(QRegExp(f->search , f->caseSensitive)) :
			msg.plainBody().contains(f->search , f->caseSensitive) )
		{
			if(f->setBG)
				msg.setBg(f->BG);
			if(f->setFG)
				msg.setFg(f->FG);
			if(f->setImportance)
				msg.setImportance((KopeteMessage::MessageImportance)f->importance);

			break; //uh?
		}
	}
}

QPtrList<Filter> HighlightPlugin::filters()
{
	return m_filters;
}

Filter* HighlightPlugin::newFilter()
{
	Filter *filtre=new Filter();
	filtre->caseSensitive=false;
	filtre->isRegExp=false;
	filtre->setImportance=false;
	filtre->importance=1;
	filtre->setBG=false;
	filtre->setFG=false;
	filtre->playSound=false;
	filtre->displayName=i18n("-New filter-");
	m_filters.append(filtre);
	return filtre;
}

void HighlightPlugin::removeFilter(Filter *f)
{
	m_filters.remove(f);
	delete f;
}


#include "highlightplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

