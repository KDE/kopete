/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@tiscalinet.be>

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
#include <qstylesheet.h>
#include <qimage.h>
#include <qdatetime.h>

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
	QString matchText;
	QString replacement;
};


class Emoticons::Private
{
public:
	QMap <QChar , QValueList<Emoticon> > emoticonMap;
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


QString Emoticons::parseEmoticons(const QString& message)  //static
{
	 return self()->parse( message );
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
		QImage image( pic );
		int width = image.width(), height = image.height();

		d->emoticonAndPicList.insert( emoticons.first() , pic);

		for ( QStringList::const_iterator it = emoticons.constBegin(), end = emoticons.constEnd();
		      it != end; ++it )
		{
			QString matchEscaped=QStyleSheet::escape(*it);
			
			Emoticon e;
			e.matchText=matchEscaped;
			e.replacement=QString::fromLatin1( "<img align=\"center\" width=\"%1\" height=\"%2\" src=\"%3\" title=\"%4\"/>" )
					.arg( QString::number( width ), QString::number( height ), pic, matchEscaped );

			d->emoticonMap[ matchEscaped[0] ].append( e );
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
		d->theme = theme;

//	kdDebug(14010) << k_funcinfo << "Called" << endl;
	d->emoticonAndPicList.clear();
	d->emoticonMap.clear();

	QDomDocument emoticonMap( QString::fromLatin1( "messaging-emoticon-map" ) );
	QString filename = KGlobal::dirs()->findResource( "emoticons",  d->theme + QString::fromLatin1( "/emoticons.xml" ) );

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


QMap<QString, QString> Emoticons::emoticonAndPicList()
{
	return d->emoticonAndPicList;
}


QString Emoticons::parse( const QString &message )
{
	// if emoticons are disabled, we do nothing
	if ( !KopetePrefs::prefs()->useEmoticons() )
		return message;

//	kdDebug(14010) << k_funcinfo << message << endl;
	
	enum LoopState { ReadyToStartAnEmoticon, SkipHTMLEntity, StillInAWord } loopState = ReadyToStartAnEmoticon;

	struct EmoticonsMatches
	{
		const Emoticon *emoticon ;
		unsigned int position; //the position we are in the emoticon text
		bool verified; //wether or not we checked that there is a whistespace at the end
		EmoticonsMatches *next;
		EmoticonsMatches(EmoticonsMatches *n , const Emoticon* e) : emoticon(e),position(1),next(n) {}
	};
	EmoticonsMatches *matches=0L;
	EmoticonsMatches *found=0L;
	EmoticonsMatches *previous;

	unsigned int message_length = message.length();  //the length of the message
	unsigned int pos;
	for(pos=0;  pos < message_length; pos++ )
	{
		QChar c=message[pos];


		//verrify emoticon that are after a space.
		if(c.isSpace() || c== '<' || c=='&')
		{
			EmoticonsMatches *it=found;
			while(it && it->position+it->emoticon->matchText.length() >= pos   )
			{
				it->verified=true;
				it=it->next;
			}
		}
		
		//skip html tags
		if(c == '<')
		{
			unsigned int p2=pos+1;
			while( p2 < message_length )
			{
				QChar c2=message[p2];
				if(c2=='>')
				{
					pos=p2;
					loopState=ReadyToStartAnEmoticon;
					break;
				}
				p2++;
			}
			if( p2 < message_length )
				continue;
		}

		EmoticonsMatches *existingMatches=matches;
		previous=0L;
		

		if(loopState == ReadyToStartAnEmoticon)
		{//try to find if an emoticon may start here.
			if(d->emoticonMap.contains(c))
			{
				QValueList<Emoticon> l=d->emoticonMap[c];
				for ( QValueList<Emoticon>::const_iterator it = l.constBegin(), end = l.constEnd();
						   it != end; ++it )
				{
					matches=new EmoticonsMatches(matches, &(*it) );
					if(!previous)
						previous=matches;
				}
				
			}
		}

		if(c.isSpace())
			loopState = ReadyToStartAnEmoticon;
		else if((c.isLetterOrNumber() || c.isSymbol() )&& loopState != SkipHTMLEntity)
			loopState = StillInAWord;

		//is it possible to continue existingMatches ?
		while(existingMatches)
		{
			if(c == existingMatches->emoticon->matchText[existingMatches->position ])
			{
				existingMatches->position++;
				if(existingMatches->position==existingMatches->emoticon->matchText.length())
				{
					EmoticonsMatches *tmp=existingMatches;
					existingMatches=existingMatches->next;
					if(previous)
						previous->next=tmp->next;
					else
						matches=tmp->next;
					tmp->position=pos-tmp->emoticon->matchText.length()+1;
					tmp->next=found;
					tmp->verified=(pos+1>=message_length);
					found=tmp;
					loopState=ReadyToStartAnEmoticon; //we are after an emoticon so we can start adding new.

					//verify emoticons that stop where it start.
					tmp=found->next;
					while(tmp && tmp->position+tmp->emoticon->matchText.length() >= found->position   )
					{
						if(tmp->position+tmp->emoticon->matchText.length() == found->position)
							tmp->verified=true;
						tmp=tmp->next;
					}

				}
				else
				{
					previous=existingMatches;
					existingMatches=existingMatches->next;
				}
			}
			else
			{ //we may remove it
				EmoticonsMatches *tmp=existingMatches;
				existingMatches=existingMatches->next;
				if(previous)
					previous->next=tmp->next;
				else
					matches=tmp->next;
				delete tmp;
			}
		}

		if(c == '&')
			loopState = SkipHTMLEntity;
		else if(loopState == SkipHTMLEntity && c==';')
			loopState = ReadyToStartAnEmoticon;

	}

	//theses matches are not finished, remove it)
	while(matches)
	{
		previous=matches;
		matches=matches->next;
		delete previous;
	}

	//search for collision and reverse
	//Items in the list found are sorted in the reversed order by the position of the terminaison of the emoticon.
	previous=0L;
	while(found)
	{
		EmoticonsMatches *previous2=found;
		EmoticonsMatches *it=found->next;

		if(!found->verified) 
		{
			found=found->next;
			delete previous2;
			continue;
		}
		while(it && found->position < it->position+it->emoticon->matchText.length())
		{
			//we only keep the bigger
			if(found->emoticon->matchText.length() > it->emoticon->matchText.length() || !it->verified )
			{
				//remove it
				previous2->next=it->next;
				delete it;
				it=previous2->next;
			}
			else
				break;
		}
		if(it && found->position < it->position+it->emoticon->matchText.length()) //the loop breaked
		{
			it=found;
			found=found->next;
			delete it;
			continue;
		}
		it=found->next;
		found->next=previous;
		previous=found;
		found=it;
	}
	found=previous;


	if(!found) //no emoticons in the message, simply return it.
		return message;

	QString result;
	pos=0;
	while(found)
	{
		result+=message.mid(pos,found->position-pos);
		pos=found->position+found->emoticon->matchText.length();
		result+=found->emoticon->replacement;
		previous=found;
		found=found->next;
		delete previous;
	}
	//kdDebug(14010) << k_funcinfo << result <<endl;

	return result+message.right(message.length()-pos);
}





} //END namesapce Kopete

#include "kopeteemoticons.moc"



// vim: set noet ts=4 sts=4 sw=4:

