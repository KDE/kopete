/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart @ kde.org>
    Copyright (c) 2005      by Engin AYDOGAN          <engin@bzzzt.biz>

   Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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
#include <q3stylesheet.h>
#include <qimage.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QMap>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kdeversion.h>

#include <set>
#include <algorithm>
#include <iterator>


/*
 * Testcases can be found in the kopeteemoticontest app in the tests/ directory.
 */


namespace Kopete {


struct Emoticons::Emoticon
{
	Emoticon(){}
	QString matchText;
	QString matchTextEscaped;
	QString	picPath;
	QString picHTMLCode;
};

/* This is the object we will store each emoticon match in */
struct Emoticons::EmoticonNode {
		const Emoticon emoticon;
		int		pos;
		EmoticonNode() : emoticon(), pos( -1 ) {}
		EmoticonNode( const Emoticon e, int p ) : emoticon( e ), pos( p ) {}
};

class Emoticons::Private
{
public:
	QMap<QChar, Q3ValueList<Emoticon> > emoticonMap;
	QMap<QString, QString> emoticonAndPicList;

	/**
	 * The current icon theme from KopetePrefs
	 */
	QString theme;


};


Emoticons *Emoticons::s_self = 0L;

Emoticons *Emoticons::self()
{
	if( !s_self )
		s_self = new Emoticons;
	return s_self;
}


QString Emoticons::parseEmoticons(const QString& message, ParseMode mode )  //static
{
	 return self()->parse( message, mode );
}

QList<Emoticons::Token> Emoticons::tokenizeEmoticons( const QString& message, ParseMode mode ) // static
{
	return self()->tokenize( message, mode );
}

QList<Emoticons::Token> Emoticons::tokenize( const QString& message, uint mode )
{
	QList<Token> result;
	if ( !KopetePrefs::prefs()->useEmoticons() )
	{
		result.append( Token( Text, message ) );
		return result;
	}

	if( ! ( mode & (StrictParse|RelaxedParse) ) )
	{
		//if none of theses two mode are selected, use the mode from the config
		mode |=  KopetePrefs::prefs()->emoticonsRequireSpaces() ? StrictParse : RelaxedParse  ;
	}

	/* previous char, in the firs iteration assume that it is space since we want
	 * to let emoticons at the beginning, the very first previous QChar must be a space. */
	QChar p = ' ';
	QChar c; /* current char */
	QChar n; /* next character after a match candidate, if strict this should be QChar::null or space */

	/* This is the EmoticonNode container, it will represent each matched emoticon */
	Q3ValueList<EmoticonNode> foundEmoticons;
	Q3ValueList<EmoticonNode>::const_iterator found;
	/* First-pass, store the matched emoticon locations in foundEmoticons */
	Q3ValueList<Emoticon> emoticonList;
	Q3ValueList<Emoticon>::const_iterator it;
	int pos;

	bool inHTMLTag = false;
	bool inHTMLLink = false;
	bool inHTMLEntity = false;
	QString needle; // search for this
	for ( pos = 0; pos < message.length(); pos++ )
	{
		c = message[ pos ];

		if ( mode & SkipHTML ) // Shall we skip HTML ?
		{
			if ( !inHTMLTag ) // Are we already in an HTML tag ?
			{
				if ( c == '<' ) { // If not check if are going into one
					inHTMLTag = true; // If we are, change the state to inHTML
					p = c;
					continue;
				}
			}
			else // We are already in a HTML tag
			{
				if ( c == '>' ) { // Check if it ends
					inHTMLTag = false;	 // If so, change the state
					if ( p == 'a' )
					{
						inHTMLLink = false;
					}
				}
				else if ( c == 'a' && p == '<' ) // check if we just entered an achor tag
				{
					inHTMLLink = true; // don't put smileys in urls
				}
				p = c;
				continue;
			}
		
			if( !inHTMLEntity )
			{ // are we
				if( c == '&' )
				{
					inHTMLEntity = true;
				}
			}
		}

		if ( inHTMLLink ) // i can't think of any situation where a link adress might need emoticons
		{
			p = c;
			continue;
		}

		if ( mode & StrictParse )
			if ( !p.isSpace() )
			{
				p = c;
				continue;
			} /* strict requires space before the emoticon */
		if ( d->emoticonMap.contains( c ) )
		{
			emoticonList = d->emoticonMap[ c ];
			bool found = false;
			for ( it = emoticonList.begin(); it != emoticonList.end(); ++it )
			{
				// If this is an HTML, then search for the HTML form of the emoticon.
				// For instance <o) => &gt;o)
				needle = ( mode & SkipHTML ) ? (*it).matchTextEscaped : (*it).matchText;
				if ( ( pos == (size_t)message.find( needle, pos ) ) )
				{
					if( mode & StrictParse )
					{
					/* check if the character after this match is space or end of string*/
						n = message[ pos + needle.length() ];
						if( !n.isSpace() &&  !n.isNull() && n!='&') break;
					}
					/* Perfect match */
					foundEmoticons.append( EmoticonNode( (*it), pos ) );
					found = true;
					/* Skip the matched emoticon's matchText */
					pos += needle.length() - 1;
					break;
				}
			}
			if( !found )
			{
				if( inHTMLEntity ){
					// If we are in an HTML entitiy such as &gt;
					int htmlEnd = message.find( ';', pos );
					// Search for where it ends
					if( htmlEnd == -1 )
					{
						// Apparently this HTML entity isn't ended, something is wrong, try skip the '&'
						// and continue
						kdDebug( 14000 ) << k_funcinfo << "Broken HTML entity, trying to recover." << endl;
						inHTMLEntity = false;
						pos++;
					}
					else 
					{
						pos = htmlEnd;
						inHTMLEntity = false;
					}
				}
			}
		} /* else no emoticons begin with this character, so don't do anything */
		p = c;
	}

	/* if no emoticons found just return the text */
	if ( foundEmoticons.isEmpty() )
	{
		result.append( Token( Text, message ) );
		return result;
	}

	/* Second-pass, generate tokens based on the matches */

	pos = 0;
	int length;

	for ( found = foundEmoticons.begin(); found != foundEmoticons.end(); ++found )
	{
		needle = ( mode & SkipHTML ) ? (*found).emoticon.matchTextEscaped : (*found).emoticon.matchText;
		if ( ( length = ( (*found).pos - pos ) ) )
		{
			result.append( Token( Text,  message.mid( pos, length ) ) );
			result.append( Token( Image, (*found).emoticon.matchTextEscaped, (*found).emoticon.picPath, (*found).emoticon.picHTMLCode ) );
			pos += length + needle.length();
		}
		else
		{
			result.append( Token( Image, (*found).emoticon.matchTextEscaped, (*found).emoticon.picPath, (*found).emoticon.picHTMLCode ) );
			pos += needle.length();
		}
	}

	if ( message.length() - pos ) // if there is remaining regular text
	{
		result.append( Token( Text, message.mid( pos ) ) );
	}

	return result;
}

Emoticons::Emoticons( const QString &theme ) : QObject( kapp, "KopeteEmoticons" )
{
//	kdDebug(14010) << "KopeteEmoticons::KopeteEmoticons" << endl;
	d=new Private;
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


Emoticons::~Emoticons(  )
{
	delete d;
}



void Emoticons::addIfPossible( const QString& filenameNoExt, const QStringList &emoticons )
{
	KStandardDirs *dir = KGlobal::dirs();
	QString pic;

	//maybe an extension was given, so try to find the exact file
	pic = dir->findResource( "emoticons", d->theme + QString::fromLatin1( "/" ) + filenameNoExt );

	if( pic.isNull() )
		pic = dir->findResource( "emoticons", d->theme + QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".mng" ) );
	if ( pic.isNull() )
		pic = dir->findResource( "emoticons", d->theme + QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".png" ) );
	if ( pic.isNull() )
		pic = dir->findResource( "emoticons", d->theme + QString::fromLatin1( "/" ) + filenameNoExt + QString::fromLatin1( ".gif" ) );

	if( !pic.isNull() ) // only add if we found one file
	{
		d->emoticonAndPicList.insert( emoticons.first() , pic);

		QPixmap p;
		QString result;

		for ( QStringList::const_iterator it = emoticons.constBegin(), end = emoticons.constEnd();
		      it != end; ++it )
		{
			QString matchEscaped=Q3StyleSheet::escape(*it);

			Emoticon e;
			e.picPath = pic;

			// We need to include size (width, height attributes)  hints in the emoticon HTML code
			// Unless we do so, ChatMessagePart::slotScrollView does not work properly and causing
			// HTMLPart not to be scrolled to the very last message.
			p.load( e.picPath );
			result = QString::fromLatin1( "<img align=\"center\" src=\"" ) + 
				  e.picPath + 
				  QString::fromLatin1( "\" title=\"" ) +
				  matchEscaped + 
				  QString::fromLatin1( "\" width=\"" ) +
				  QString::number( p.width() ) +
				  QString::fromLatin1( "\" height=\"" ) +
				  QString::number( p.height() ) +
				  QString::fromLatin1( "\" />" );

			e.picHTMLCode = result;
			e.matchTextEscaped = matchEscaped;
			e.matchText = *it;
			d->emoticonMap[ matchEscaped[0] ].append( e );
			d->emoticonMap[ (*it)[0] ].append( e );
		}
	}
}

void Emoticons::initEmoticons( const QString &theme )
{
	if(theme.isNull())
	{
		if ( d->theme == KopetePrefs::prefs()->iconTheme() )
			return;

		d->theme = KopetePrefs::prefs()->iconTheme();
	}
	else
	{
		d->theme = theme;
	}

//	kdDebug(14010) << k_funcinfo << "Called" << endl;
	d->emoticonAndPicList.clear();
	d->emoticonMap.clear();

	QDomDocument emoticonMap( QString::fromLatin1( "messaging-emoticon-map" ) );

    QString filename= KGlobal::dirs()->findResource( "emoticons",  d->theme + QString::fromLatin1( "/emoticons.xml" ) );

	QFile mapFile( filename );
	mapFile.open( QIODevice::ReadOnly );
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


QMap<QString, QString> Emoticons::emoticonAndPicList()
{
	return d->emoticonAndPicList;
}


QString Emoticons::parse( const QString &message, ParseMode mode )
{
	if ( !KopetePrefs::prefs()->useEmoticons() )
                return message;

	QList<Token> tokens = tokenize( message, mode );
	QString result;
	QPixmap p;
	foreach (Token token , tokens )
	{
		switch ( token.type )
		{
		case Text:
			result += token.text;
		break;
		case Image:
			result += (*token).picHTMLCode;
			kdDebug( 14010 ) << k_funcinfo << "Emoticon html code: " << result << endl;
		break;
		default:
			kdDebug( 14010 ) << k_funcinfo << "Unknown token type. Something's broken." << endl;
		}
	}
	return result;
}

} //END namesapce Kopete

#include "kopeteemoticons.moc"



// vim: set noet ts=4 sts=4 sw=4:

