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
#include <qregexp.h>
#include <qstylesheet.h>
#include <qimage.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>

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

void KopeteEmoticons::addIfPossible( const QString& filenameNoExt, QStringList emoticons )
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
//		kdDebug(14010) << "KopeteEmoticons::addIfPossible : found pixmap for emoticons" <<endl;
		map[pic] = emoticons;
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
	map.clear(); // empty the mapping

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
	EmoticonMap::Iterator it;
	for ( it = map.begin(); it != map.end(); ++it )
	{
		// search in QStringList data for emoticon
		if ( it.data().findIndex(em) != -1 )
			return it.key();
		// if found return path for corresponding animation or pixmap
	}

	return QString();
}

QStringList KopeteEmoticons::picPathToEmoticon ( const QString& path )
{
	EmoticonMap::Iterator it = map.find( path );
	if ( it != map.end() )
		return it.data();

	return QStringList();
}

QStringList KopeteEmoticons::emoticonList()
{
	QStringList retVal;
	EmoticonMap::Iterator it;

	for ( it = map.begin(); it != map.end(); ++it )
		retVal += it.data();

	return retVal;
}

QStringList KopeteEmoticons::picList()
{
	QStringList retVal;
	EmoticonMap::Iterator it;

	for ( it = map.begin(); it != map.end(); ++it )
		retVal += it.key();

	return retVal;
}


QMap<QString, QString> KopeteEmoticons::emoticonAndPicList()
{
	QMap<QString, QString> retVal;
	EmoticonMap::Iterator it;

	for ( it = map.begin(); it != map.end(); ++it )
		retVal[it.data().first()] = it.key();

	return retVal;
}

QString KopeteEmoticons::parseEmoticons( QString message )
{
	// if emoticons are disabled, we do nothing
	if ( !KopetePrefs::prefs()->useEmoticons() )
		return message;

	QStringList emoticons = KopeteEmoticons::emoticons()->emoticonList();

	/*
	 * FIXME: do proper emoticon support
	 *
	 * Testcases can be found in the kopeteemoticontest app in the tests/ directory.
	 */
	
	for ( QStringList::Iterator it = emoticons.begin(); it != emoticons.end(); ++it )
	{
		QString escaped = QStyleSheet::escape( *it );
		if ( message.contains( escaped ) )
		{
			QString imgPath = KopeteEmoticons::emoticons()->emoticonToPicPath( *it );
			QImage iconImage( imgPath );

			// Due to the nature of the regexp replacing and the lack of lookbacks in
			// QRegExp we need a workaround for URIs like http://www.kde.org, as these
			// would replace the ':/' part with an emoticon.
			// The workaround is to add a special condition that the pattern should
			// not be followed by a '/' if it already ends with a slash itself.
			QString doubleSlashPattern;
			if ( escaped.endsWith( QString::fromLatin1( "/" ) ) )
				doubleSlashPattern = QString::fromLatin1( "(?!/)" );

			// Explanation of the below regexp:
			// "(?![^<]+>)"              - Negative lookahead for "[^<]+>", i.e. don't match when the emoticon
			//                             pattern is directly preceded by an HTML tag.
			//                             FIXME: What's the use of this part? Testcase? - Martijn
			// "(?!(https?://|mailto:))" - Another negative lookahead. Don't match if there is a protocol URI
			//                             directly in front of the emoticon
			// "(%1)"                    - The actual emoticon pattern
			// "(?![^<]+>)"              - Last lookahead: don't match if the emoticon pattern is part of an
			//                             HTML tag.
			QRegExp regex = QRegExp( QString::fromLatin1( "(?![^<]+>)(?!(https?://|mailto:))(%1)" ).arg( QRegExp::escape( escaped ) ) +
				doubleSlashPattern + QString::fromLatin1( "(?![^<]+>)" ) );

			message.replace( regex,
				QString::fromLatin1( "<img align=\"center\" width=\"" ) +
				QString::number( iconImage.width() ) +
				QString::fromLatin1( "\" height=\"" ) +
				QString::number( iconImage.height() ) +
				QString::fromLatin1( "\" src=\"" ) + imgPath +
				QString::fromLatin1( "\" title=\"" ) + escaped +
				QString::fromLatin1( "\"/>" ) );
			// FIXME: It would seem to me a more correct way of doing things
			// would be to only analyze non-special fields.  Any special field
			// should be pre-escaped and not considered in emoticon parsing.
			// Such fields include: URLs, Contact IDs, Display Aliases,
			// Metacontact names, Status/system messages, HTML tags/entities.
			// - Casey
		}
	}

	return message;
}

#include "kopeteemoticons.moc"

// vim: set noet ts=4 sts=4 sw=4:

