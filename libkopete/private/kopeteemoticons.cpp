/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002      by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteemoticons.h"

#include "kopeteprefs.h"

#include <qdom.h>
#include <qfile.h>
#include <qstylesheet.h>
#include <qimage.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <set>
#include <algorithm>
#include <iterator>

/*
 * FIXME: do proper emoticon support
 *
 * Testcases can be found in the kopeteemoticontest app in the tests/ directory.
 */

// QValueList requires this due to a poorly-thought-out implementation
Emoticon::Emoticon()
{
}

Emoticon::Emoticon( const QString &filename, const QString &matchText )
: m_filename(filename), m_matchText(matchText), m_matchTextEscaped(QStyleSheet::escape(matchText))
{
	QImage image( filename );
	int width = image.width(), height = image.height();

	// Due to the nature of the regexp replacing and the lack of lookbacks in
	// QRegExp we need a workaround for URIs like http://www.kde.org, as these
	// would replace the ':/' part with an emoticon.
	// The workaround is to add a special condition that the pattern should
	// not be followed by a '/' if it already ends with a slash itself.
	QString endLookahead = QString::fromLatin1( "(?![^<]+>)" );
	if ( m_matchTextEscaped.endsWith( QString::fromLatin1( "/" ) ) )
		endLookahead += QString::fromLatin1( "(?!/)" );
	
	// Explanation of the below regexp:
	// "(?![^<]+>)"              - Negative lookahead for "[^<]+>", i.e. don't match when the emoticon
	//                             pattern is directly preceded by an HTML tag.
	//                             FIXME: What's the use of this part? Testcase? - Martijn
	// "(?!(https?://|mailto:))" - Another negative lookahead. Don't match if there is a protocol URI
	//                             directly in front of the emoticon
	// "(%1)"                    - The actual emoticon pattern
	// "(?![^<]+>)"              - Last lookahead: don't match if the emoticon pattern is part of an
	//                             HTML tag.
	m_regExp = QRegExp( QString::fromLatin1( "(?![^<]+>)(?!(https?://|mailto:))(%1)%2" )
	                  .arg( QRegExp::escape( m_matchTextEscaped ), endLookahead ) );
	m_replacement = QString::fromLatin1( "<img align=\"center\" width=\"%1\" height=\"%2\" src=\"%3\" title=\"%4\"/>" )
		.arg( QString::number( width ), QString::number( height ), m_filename, m_matchTextEscaped );
	
	// FIXME: It would seem to me a more correct way of doing things
	// would be to only analyze non-special fields.  Any special field
	// should be pre-escaped and not considered in emoticon parsing.
	// Such fields include: URLs, Contact IDs, Display Aliases,
	// Metacontact names, Status/system messages, HTML tags/entities.
	// - Casey
}

void Emoticon::parse( QString &message ) const
{
	if ( message.contains( m_matchTextEscaped ) )
		message.replace( m_regExp, m_replacement );
}

KopeteEmoticons *KopeteEmoticons::s_instance = 0L;

KopeteEmoticons *KopeteEmoticons::emoticons()
{
	if( !s_instance )
		s_instance = new KopeteEmoticons;
	return s_instance;
}

KopeteEmoticons::KopeteEmoticons( const QString &theme ) : QObject( kapp, "KopeteEmoticons" )
{
//	kdDebug(14010) << "KopeteEmoticons::KopeteEmoticons" << endl;
	if(theme.isNull())
	{
		initEmoticons();
		connect( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(initEmoticons()) );
	}
	else
	{
		initEmoticons( theme );
	}
}

void KopeteEmoticons::addIfPossible( const QString& filenameNoExt, const QStringList &emoticons )
{
	KStandardDirs *dir = KGlobal::dirs();
	QString pic;

	//maybe an extension was given, so try to find the exact file
	pic = dir->findResource( "data", QString::fromLatin1( "kopete/pics/emoticons/" ) + m_theme +
		QString::fromLatin1( "/" ) + filenameNoExt );

	if( pic.isNull() )
		pic = dir->findResource( "data", QString::fromLatin1( "kopete/pics/emoticons/" ) + m_theme +
			QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".mng" ) );
	if ( pic.isNull() )
		pic = dir->findResource( "data", QString::fromLatin1( "kopete/pics/emoticons/" ) +
			m_theme + QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".png" ) );
	if ( pic.isNull() )
		pic = dir->findResource( "data", QString::fromLatin1( "kopete/pics/emoticons/" ) +
			m_theme + QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".gif" ) );

	if( !pic.isNull() ) // only add if we found one file
	{
		for ( QStringList::const_iterator it = emoticons.constBegin(), end = emoticons.constEnd();
		      it != end; ++it )
		{
			m_emoticons.push_back( Emoticon(pic, *it) );
		}
	}
}

void KopeteEmoticons::initEmoticons( const QString &theme )
{
	if(theme.isNull())
	{
		if ( m_theme == KopetePrefs::prefs()->iconTheme() )
			return;

		m_theme = KopetePrefs::prefs()->iconTheme();
	}
	else
		m_theme = theme;

//	kdDebug(14010) << k_funcinfo << "Called" << endl;
	m_emoticons.clear();

	QDomDocument emoticonMap( QString::fromLatin1( "messaging-emoticon-map" ) );
	QString filename = KGlobal::dirs()->findResource( "data", QString::fromLatin1( "kopete/pics/emoticons/" ) +
		m_theme + QString::fromLatin1( "/emoticons.xml" ) );

	if( filename.isEmpty() )
	{
		kdDebug(14010) << "KopeteEmoticons::initEmoticons : WARNING: emoticon-map not found" <<endl;
		return ;
	}

	QFile mapFile( filename );
	mapFile.open( IO_ReadOnly );
	emoticonMap.setContent( &mapFile );

	QDomElement list = emoticonMap.documentElement();
	QDomNode node = list.firstChild();
	while( !node.isNull() )
	{
		QDomElement element = node.toElement();
		if( !element.isNull() )
		{
			if( element.tagName() == QString::fromLatin1( "emoticon" ) )
			{
				QString emoticon_file = element.attribute(
					QString::fromLatin1( "file" ), QString::null );
				QStringList items;

				QDomNode emoticonNode = node.firstChild();
				while( !emoticonNode.isNull() )
				{
					QDomElement emoticonElement = emoticonNode.toElement();
					if( !emoticonElement.isNull() )
					{
						if( emoticonElement.tagName() == QString::fromLatin1( "string" ) )
						{
							items << emoticonElement.text();
						}
						else
						{
							kdDebug(14010) << k_funcinfo <<
								"Warning: Unknown element '" << element.tagName() <<
								"' in emoticon data" << endl;
						}
					}
					emoticonNode = emoticonNode.nextSibling();
				}

				addIfPossible ( emoticon_file, items );
			}
			else
			{
				kdDebug(14010) << k_funcinfo << "Warning: Unknown element '" <<
					element.tagName() << "' in map file" << endl;
			}
		}
		node = node.nextSibling();
	}
	mapFile.close();
}

QString KopeteEmoticons::emoticonToPicPath ( const QString& em )
{
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		// search in QStringList data for emoticon
		if ( (*it).matchText() == em )
			return (*it).filename();
		// if found return path for corresponding animation or pixmap
	}

	return QString::null;
}

QStringList KopeteEmoticons::picPathToEmoticon ( const QString& path )
{
	QStringList result;
	
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		if ( (*it).filename() == path )
			result += (*it).matchText();
	}
	
	return result;
}

QStringList KopeteEmoticons::emoticonList()
{
	QStringList result;
	
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		result += (*it).matchText();
	}

	return result;
}

struct KopeteEmoticonsQStringCompare
{
	bool operator()(const QString &a, const QString &b) { return QString::compare(a,b); }
};

QStringList KopeteEmoticons::picList()
{
	std::set<QString, KopeteEmoticonsQStringCompare> files;
	
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		files.insert( (*it).filename() );
	}
	
	QStringList result;
	std::copy( files.begin(), files.end(), std::back_inserter(result) );
	return result;
}


QMap<QString, QString> KopeteEmoticons::emoticonAndPicList()
{
	QMap<QString, QString> result;
	
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		result[(*it).matchText()] = (*it).filename();
	}
	
	return result;
}

QString KopeteEmoticons::parse( const QString &message )
{
	// if emoticons are disabled, we do nothing
	if ( !KopetePrefs::prefs()->useEmoticons() )
		return message;

	QString result = message;
	for ( EmoticonList::const_iterator it = m_emoticons.constBegin(), end = m_emoticons.constEnd();
	      it != end; ++it )
	{
		(*it).parse( result );
	}
	return result;
}

#include "kopeteemoticons.moc"

// vim: set noet ts=4 sts=4 sw=4:

