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
#include <qdir.h>

#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <knotifyclient.h>


#include "kopetemessagemanagerfactory.h"

#include "filter.h"
#include "highlightplugin.h"
#include "highlightpreferences.h"


K_EXPORT_COMPONENT_FACTORY( kopete_highlight, KGenericFactory<HighlightPlugin> );

HighlightPlugin::HighlightPlugin( QObject *parent, const char *name, const QStringList &/*args*/ )
: KopetePlugin( KGlobal::instance(), parent, name )
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), SLOT( slotIncomingMessage( KopeteMessage & ) ) );

	load();

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


void HighlightPlugin::slotIncomingMessage( KopeteMessage& msg )
{
	if(msg.direction() != KopeteMessage::Inbound)
		return;	// FIXME: highlighted internal/actions messages are not showed correctly in the chat window (bad style)
				//  but they should maybe be highlinghted if needed

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
			if(f->playSound)
				KNotifyClient::userEvent (QString::null, KNotifyClient::Sound, KNotifyClient::Default, f->soundFN );

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

void HighlightPlugin::save()
{
	QString fileName = locateLocal( "appdata", QString::fromLatin1( "highlight.xml" ) );

	KSaveFile file( fileName );
	if( file.status() == 0 )
	{
		QTextStream *stream = file.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );

		QString xml = QString::fromLatin1(
			"<?xml version=\"1.0\"?>\n"
			"<!DOCTYPE kopete-highlight-plugin>\n"
			"<highlight-plugin>\n" );

			// Save metafilter information.
		QPtrListIterator<Filter> filtreIt( m_filters );
		for( ; filtreIt.current(); ++filtreIt )
		{
			Filter *filtre = *filtreIt;
			xml += QString::fromLatin1( "  <filter>\n    <display-name>" )
				+ QStyleSheet::escape(filtre->displayName)
				+ QString::fromLatin1( "</display-name>\n" );

			xml += QString::fromLatin1("    <search caseSensitive=\"") + QString::number( static_cast<int>( filtre->caseSensitive ) ) +
				QString::fromLatin1("\" regExp=\"") + QString::number( static_cast<int>( filtre->isRegExp ) ) +
				QString::fromLatin1( "\">" ) + QStyleSheet::escape( filtre->search ) + QString::fromLatin1( "</search>\n" );

			xml += QString::fromLatin1("    <BG set=\"") + QString::number( static_cast<int>( filtre->setBG ) ) +
				QString::fromLatin1( "\">" ) + QStyleSheet::escape( filtre->BG.name() ) + QString::fromLatin1( "</BG>\n" );
			xml += QString::fromLatin1("    <FG set=\"") + QString::number( static_cast<int>( filtre->setFG ) ) +
				QString::fromLatin1( "\">" ) + QStyleSheet::escape( filtre->FG.name() ) + QString::fromLatin1( "</FG>\n" );

			xml += QString::fromLatin1("    <importance set=\"") + QString::number( static_cast<int>( filtre->setImportance ) ) +
				QString::fromLatin1( "\">" ) + QString::number( filtre->importance ) + QString::fromLatin1( "</importance>\n" );

			xml += QString::fromLatin1("    <sound set=\"") + QString::number( static_cast<int>( filtre->playSound ) ) +
				QString::fromLatin1( "\">" ) + QStyleSheet::escape( filtre->soundFN ) + QString::fromLatin1( "</sound>\n" );

			xml += QString::fromLatin1( "  </filter>\n" );
		}

		xml += QString::fromLatin1( "</highlight-plugin>\n" );

		*stream << xml;
	}

}

void HighlightPlugin::load()
{
	QString filename = locateLocal( "appdata", QString::fromLatin1( "highlight.xml" ) );
	if( filename.isEmpty() )
		return ;

	QDomDocument filterList( QString::fromLatin1( "highlight-plugin" ) );

	QFile filterListFile( filename );
	filterListFile.open( IO_ReadOnly );
	filterList.setContent( &filterListFile );

	QDomElement list = filterList.documentElement();

	QDomNode node = list.firstChild();
	while( !node.isNull() )
	{
		QDomElement element = node.toElement();
		if( !element.isNull() )
		{
//			if( element.tagName() == QString::fromLatin1("filter")
//			{
				Filter *filtre=newFilter();
				QDomNode filterNode = node.firstChild();

				while( !filterNode.isNull() )
				{
					QDomElement filterElement = filterNode.toElement();
					if( !filterElement.isNull() )
					{
						if( filterElement.tagName() == QString::fromLatin1( "display-name" ) )
						{
							filtre->displayName = filterElement.text();
						}
						else if( filterElement.tagName() == QString::fromLatin1( "search" ) )
						{
							filtre->search = filterElement.text();

							filtre->caseSensitive= ( filterElement.attribute( QString::fromLatin1( "caseSensitive" ), QString::fromLatin1( "1" ) ) == QString::fromLatin1( "1" ) );
							filtre->isRegExp= ( filterElement.attribute( QString::fromLatin1( "regExp" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
						}
						else if( filterElement.tagName() == QString::fromLatin1( "FG" ) )
						{
							filtre->FG = filterElement.text();
							filtre->setFG= ( filterElement.attribute( QString::fromLatin1( "set" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
						}
						else if( filterElement.tagName() == QString::fromLatin1( "BG" ) )
						{
							filtre->BG = filterElement.text();
							filtre->setBG= ( filterElement.attribute( QString::fromLatin1( "set" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
						}
						else if( filterElement.tagName() == QString::fromLatin1( "importance" ) )
						{
							filtre->importance = filterElement.text().toUInt();
							filtre->setImportance= ( filterElement.attribute( QString::fromLatin1( "set" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
						}
						else if( filterElement.tagName() == QString::fromLatin1( "sound" ) )
						{
							filtre->soundFN = filterElement.text();
							filtre->playSound = ( filterElement.attribute( QString::fromLatin1( "set" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
						}
					}
					filterNode = filterNode.nextSibling();
				}
//			}
		}
		node = node.nextSibling();
	}
	filterListFile.close();

}




#include "highlightplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

