/*
    highlightconfig.cpp

    Copyright (c) 2003      by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2003      by Matt Rogers           <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qfile.h>
#include <qstylesheet.h>
#include <qregexp.h>
#include <qdir.h>
#include <qdom.h>

#include <ksavefile.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "filter.h"
#include "highlightconfig.h"


HighlightConfig::HighlightConfig()
{
	load();
	m_filters.setAutoDelete(true);
}

HighlightConfig::~HighlightConfig()
{
	m_filters.clear();
}

void HighlightConfig::removeFilter(Filter *f)
{
    //m_filters is "autodelete (true) so when we use remove(...) it deleted f
    //so don't use (delete (f) after otherwise ot crash
	m_filters.remove(f);
}

void HighlightConfig::appendFilter(Filter *f)
{
	m_filters.append(f);
}

QPtrList<Filter> HighlightConfig::filters() const
{
	return m_filters;
}

Filter* HighlightConfig::newFilter()
{
	Filter *filtre=new Filter();
	filtre->caseSensitive=false;
	filtre->isRegExp=false;
	filtre->setImportance=false;
	filtre->importance=1;
	filtre->setBG=false;
	filtre->setFG=false;
	filtre->playSound=false;
	filtre->raiseView=false;
	filtre->displayName=i18n("-New filter-");
	m_filters.append(filtre);
	return filtre;
}

void HighlightConfig::load()
{
	m_filters.clear(); //clear filters

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
						else if( filterElement.tagName() == QString::fromLatin1( "raise" ) )
						{
							filtre->raiseView = ( filterElement.attribute( QString::fromLatin1( "set" ), QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
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

void HighlightConfig::save()
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

			xml += QString::fromLatin1("    <raise set=\"") + QString::number( static_cast<int>( filtre->raiseView ) ) +
				QString::fromLatin1( "\"></raise>\n" );

			xml += QString::fromLatin1( "  </filter>\n" );
		}

		xml += QString::fromLatin1( "</highlight-plugin>\n" );

		*stream << xml;
	}
}

// vim: set noet ts=4 sts=4 sw=4:
