/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "config.h"
#ifdef HAVE_TGMATH_H
#include <tgmath.h>
#endif

#include <qdatetime.h>
#include <qfont.h>
#include <qstylesheet.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kopeteemoticons.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteprefs.h"

class KopeteMessagePrivate
{
public:
	uint refCount;

	const KopeteContact *from;
	KopeteContactPtrList to;
	QColor bgColor;
	QColor fgColor;
	QColor contactColor;

	QDateTime timeStamp;
	QFont font;
	QString body;
	QString subject;
	KopeteMessage::MessageDirection direction;
	KopeteMessage::MessageFormat format;
	KopeteMessage::MessageType type;

	bool bgOverride;
	bool highlighted;
};

KopeteMessage::KopeteMessage()
{
	d = new KopeteMessagePrivate;

	init( QDateTime::currentDateTime(), 0L, KopeteContactPtrList(), QString::null, QString::null, Internal, PlainText, Chat );
}

KopeteMessage::KopeteMessage( const KopeteContact *fromKC, const KopeteContactPtrList &toKC, const QString &body,
	MessageDirection direction, MessageFormat f, MessageType type )
{
	d = new KopeteMessagePrivate;

	init( QDateTime::currentDateTime(), fromKC, toKC, body, QString::null, direction, f, type );
}

KopeteMessage::KopeteMessage( const KopeteContact *fromKC, const KopeteContactPtrList &toKC, const QString &body,
	const QString &subject, MessageDirection direction, MessageFormat f, MessageType type )
{
	d = new KopeteMessagePrivate;

	init( QDateTime::currentDateTime(), fromKC, toKC, body, subject, direction, f, type );
}

KopeteMessage::KopeteMessage( const QDateTime &timeStamp, const KopeteContact *fromKC, const KopeteContactPtrList &toKC,
	const QString &body, MessageDirection direction, MessageFormat f, MessageType type )
{
	d = new KopeteMessagePrivate;

	init( timeStamp, fromKC, toKC, body, QString::null, direction, f, type );
}

KopeteMessage::KopeteMessage( const QDateTime &timeStamp, const KopeteContact *fromKC, const KopeteContactPtrList &toKC,
	const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, MessageType type )
{
	d = new KopeteMessagePrivate;

	init( timeStamp, fromKC, toKC, body, subject, direction, f, type );
}

KopeteMessage::KopeteMessage( const KopeteMessage &other )
{
	d = other.d;
	d->refCount++;
}

KopeteMessage& KopeteMessage::operator=( const KopeteMessage &other )
{
	detach();
	delete d;

	d = other.d;
	d->refCount++;

	return *this;
}

KopeteMessage::~KopeteMessage()
{
	d->refCount--;
	if( !d->refCount )
		delete d;
}

void KopeteMessage::setBgOverride( bool enabled )
{
	detach();
	d->bgOverride = enabled;
}

void KopeteMessage::setFg( const QColor &color )
{
	detach();
	d->fgColor = color;
	compareColors( d->fgColor, d->bgColor );
}

void KopeteMessage::setBg( const QColor &color )
{
	detach();
	d->bgColor = color;
	compareColors( d->fgColor, d->bgColor );
	compareColors( d->contactColor, d->bgColor );
}

void KopeteMessage::compareColors( QColor &colorFg, QColor &colorBg )
{
	int h1, s1, v1, h2, s2, v2, vDiff;
	colorFg.hsv( &h1, &s1, &v1 );
	colorBg.hsv( &h2, &s2, &v2 );
	vDiff = v1 - v2;

	if( h1 == s1 && h2 == s2 && ( abs((double)vDiff) <= 150 ) )
		colorFg = QColor( h2, s2, (v1 + 127) % 255, QColor::Hsv );
}

void KopeteMessage::setFont( const QFont &font )
{
	detach();
	d->font = font;
}

void KopeteMessage::highlight()
{
	setBody( QString::fromLatin1("<span style=\"background-color:%1;color:%2\">%3</span>")
		.arg( KopetePrefs::prefs()->highlightBackground().name() )
		.arg( KopetePrefs::prefs()->highlightForeground().name() )
		.arg( escapedBody() ), RichText );

	setBg( KopetePrefs::prefs()->highlightBackground() );
	setFg( KopetePrefs::prefs()->highlightForeground() );

	d->highlighted = true;
}

bool KopeteMessage::isHighlighted()
{
	return d->highlighted;
}

void KopeteMessage::setBody( const QString &body, MessageFormat f )
{
	detach();
	/*if( d->direction == Outbound && body.startsWith( QString::fromLatin1( "/me " ) ) )
	{
		d->body = body.section( QString::fromLatin1( " " ), 1 ).prepend(
			QString::fromLatin1( " " ) ).prepend( d->from->displayName() ).prepend( QString::fromLatin1( "*" ) );
	}
	else
	{*/
		d->body = body;
	//}

	d->format = f;
}

void KopeteMessage::init( const QDateTime &timeStamp, const KopeteContact *from, const KopeteContactPtrList &to,
	const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, MessageType type )
{
	static QMap<QString,QColor> colorMap;
	static int lastColor;

	d->refCount = 1;
	d->timeStamp = timeStamp;
	d->from = from;
	d->to   = to;
	d->subject = subject;
	d->direction = direction;
	d->fgColor = QColor();
	d->bgColor = QColor();
	d->font = QFont();
	setBody( body, f );
	d->bgOverride = false;
	d->type = type;
	d->highlighted = false;

	if( from )
	{
		QString fromName = d->from->metaContact() ? d->from->metaContact()->displayName() : d->from->displayName();

		if( !colorMap.contains( fromName ) )
		{
			QColor newColor;
			if( direction == Outbound )
				newColor = Qt::yellow;
			else
			{
				switch( (lastColor++) % 10 )
				{
					case 0:
						newColor = Qt::red;
						break;
					case 1:
						newColor =  Qt::green;
						break;
					case 2:
						newColor =  Qt::blue;
						break;
					case 3:
						newColor =  Qt::cyan;
						break;
					case 4:
						newColor =  Qt::magenta;
						break;
					case 5:
						newColor =  Qt::darkRed;
						break;
					case 6:
						newColor =  Qt::darkGreen;
						break;
					case 7:
						newColor =  Qt::darkCyan;
						break;
					case 8:
						newColor =  Qt::darkMagenta;
						break;
					case 9:
						newColor =  Qt::darkYellow;
						break;
				}
			}

			colorMap.insert( fromName, newColor );
		}

		d->contactColor = colorMap[ fromName ];
	}
}

QString KopeteMessage::plainBody() const
{
	if( d->format == PlainText )
		return d->body;

	//FIXME: is there a better way to unescape HTML?
	QString r = d->body;
	r = r.replace( QRegExp( QString::fromLatin1( "<br/>" ) ), QString::fromLatin1( "\n" ) ).
		replace( QRegExp( QString::fromLatin1( "<br>" ) ), QString::fromLatin1( "\n" ) ).
		replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::fromLatin1( "" ) ).
		replace( QRegExp( QString::fromLatin1( "&gt;" ) ), QString::fromLatin1( ">" ) ).
		replace( QRegExp( QString::fromLatin1( "&lt;" ) ), QString::fromLatin1( "<" ) ).
		replace( QRegExp( QString::fromLatin1( "&nbsp;" ) ), QString::fromLatin1( " " ) ).
		replace( QRegExp( QString::fromLatin1( "&amp;" ) ), QString::fromLatin1( "&" ) );

	return r;
}

QString KopeteMessage::escapedBody() const
{
	if( d->format == PlainText )
	{
		QStringList words;
		QString parsedString;

		//Strip whitespace off the end of the string only
		//(stripWhiteSpace removes it from beginning as well)
		int stringEnd = d->body.findRev( QRegExp( QString::fromLatin1( "\\S" ) ) );
//		kdDebug(14010) << k_funcinfo << "String End:" << stringEnd <<endl;
		if( stringEnd > -1 )
			parsedString = QStyleSheet::escape( d->body.left( stringEnd + 1 ) );
		else
			parsedString = QStyleSheet::escape( d->body );

		words = QStringList::split( ' ', parsedString, true );

		// Replace multiple spaces with '&nbsp;', but leave the first space
		// intact for any necessary wordwrap:
		parsedString = "";
		for( QStringList::Iterator it = words.begin(); it != words.end(); ++it )
		{
			if( ( *it ).isEmpty() )
				parsedString += QString::fromLatin1( "&nbsp;" );
			else
				parsedString += *it + QString::fromLatin1( " " );
		}

		//Replace carriage returns inside the text
		parsedString = parsedString.replace( QRegExp( QString::fromLatin1( "\n" ) ), QString::fromLatin1( "<br/>" ) );

		//Replace a tab with 4 spaces
		parsedString = parsedString.replace( QRegExp( QString::fromLatin1( "\t" ) ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

//		kdDebug(14010) << k_funcinfo << parsedString <<endl;
		return parsedString;
	}

	return d->body;
}

QString KopeteMessage::parsedBody() const
{
	if( d->format == ParsedHTML )
		return d->body;

	return KopeteEmoticons::parseEmoticons(parseHTML(escapedBody()));
}

QString KopeteMessage::transformMessage( const QString &model ) const
{
	QString message;
	bool F_first = true;
	bool L_first = true;
	unsigned int f = 0;

	do
	{
		QChar c = model[ f ];
		if( c != '%' )
		{
			message += c;
		}
		else
		{
			f++;
			c = model[ f ];
			// Using latin1 is safe, we don't check for other chars, and latin1() returns non-latin as '\0'
			switch( c.latin1() )
			{
				case 'M':  //insert Message
					message.append( parsedBody() );
					break;

				case 'T':  //insert Timestamp
					message.append( KGlobal::locale()->formatTime(d->timeStamp.time(), true) );
					break;

				case 'F':  //insert Fonts
					if( F_first ) // <font>....
					{
						message += QString::fromLatin1( "<font" );
						if ( d->fgColor.isValid() )
							message += QString::fromLatin1( " color=\"" ) + d->fgColor.name() + QString::fromLatin1( "\"" );
						if ( d->font != QFont() )
							message += QString::fromLatin1( " face=\"" ) + d->font.family() + QString::fromLatin1( "\"" );
						message += QString::fromLatin1( ">" );
						if ( d->font != QFont() && d->font.bold())
							message += QString::fromLatin1( "<b>" );
						if ( d->font != QFont() && d->font.italic())
							message += QString::fromLatin1( "<i>" );
						F_first=false;
					}
					else            // </font>
					{
						if ( d->font != QFont() && d->font.italic())
							message += QString::fromLatin1( "</i>" );
						if ( d->font != QFont() && d->font.bold())
							message += QString::fromLatin1( "</b>" );

						message += QString::fromLatin1( "</font>" );
						F_first=true;
					}
					break;

				case 'L':  //insert Contact color
					if( L_first ) // <font>....
					{
						message += QString::fromLatin1( "<font color=\"%1\">" ).arg( d->contactColor.name() );
						L_first = false;
					}
					else            // </font>
					{
						message += QString::fromLatin1( "</font>" );
						L_first = true;
					}
					break;

				case 'b':   //BgColor
					if ( d->bgColor.isValid() && !d->bgOverride )
						message += d->bgColor.name();
					break;

				case 'i': //only inbound
					if(d->direction != Inbound)
					{
						bool b=false;
						bool fin;
						do
						{
							fin=false;
							f++;
							QChar c2 = model[ f ];
							if(c2=='i' && b)
								fin=true;
							b = (c2=='%');
						}
						while( f < model.length() && !fin );
					}
					break;

				case 'o': //only outbound
					if(d->direction != Outbound)
					{
						bool b=false;
						bool fin;
						do
						{
							fin=false;
							f++;
							QChar c2 = model[ f ];
							if(c2=='o' && b)
								fin=true;
							b=(c2=='%');
						}
						while( f < model.length() && !fin );
					}
					break;
				case 's': //only internal
					if(d->direction != Internal)
					{
						bool b=false;
						bool fin;
						do
						{
							fin=false;
							f++;
							QChar c2 = model[ f ];
							if(c2=='s' && b)
								fin=true;
							b=(c2=='%');
						}
						while( f < model.length() && !fin );
					}
					break;
				case 'a': //action
					if(d->direction != Action)
					{
						bool b=false;
						bool fin;
						do
						{
							fin=false;
							f++;
							QChar c2 = model[ f ];
							if(c2=='a' && b)
								fin=true;
							b=(c2=='%');
						}
						while( f < model.length() && !fin );
					}
					break;
				case 'e': //not internal (external)
					if( (d->direction == Internal) || (d->direction == Action) )
					{
						bool b=false;
						bool fin;
						do
						{
							fin=false;
							f++;
							QChar c2 = model[ f ];
							if(c2=='e' && b)
								fin=true;
							b=(c2=='%');
						}
						while( f < model.length() && !fin );
					}
					break;

				case 'f': //insert the 'from' metaContact's displayName
					if (d->from->metaContact())
						message.append( QStyleSheet::escape(d->from->metaContact()->displayName()) );
					else
						message.append( QStyleSheet::escape(d->from->displayName()) );
					break;

				case 't': //insert the 'to' metaContact's displayName
					if (to().first()->metaContact())
						message.append( QStyleSheet::escape(to().first()->metaContact()->displayName()) );
					else
						message.append( QStyleSheet::escape(to().first()->displayName()) );
					break;
					
				case 'c': //the 'from' KopeteContact displayName
					if (to().first())
						message.append( QStyleSheet::escape(to().first()->displayName()) );
					break;
					
				case 'C': //the 'to' KopeteContact displayName
					if (d->from)
						message.append( QStyleSheet::escape(d->from->displayName()) );
					break;

				case 'I': //insert the statusicon path
					if(d->from)
					{
						//FIXME -Will
						QString icoPath = KGlobal::iconLoader()->iconPath( d->from->onlineStatus().overlayIcon(), KIcon::Small );
						if (!icoPath.isNull())
						message.append( QStyleSheet::escape(icoPath) );
					}
					break;
				default:
					message += c;
					break;
			}
		}
		f++;
	}
	while( f < model.length() );

	return message;
}

QString KopeteMessage::parseHTML( const QString &message, bool parseURLs )
{
	QString text, result;
	QRegExp regExp;
	uint len = message.length();
	int matchLen;
	unsigned int startIdx;
	int lastReplacement = -1;
	text = message;

	for ( uint idx=0; idx<len; idx++ )
	{
		switch( text[idx].latin1() )
		{
			case '\r':
				lastReplacement = idx;
				break;
			case '@':		// email-addresses or message-ids
			{
				if ( parseURLs )
				{
					startIdx = idx;
					while (
						(startIdx>(uint)(lastReplacement+1)) &&
						(text[startIdx-1]!=' ') &&
						(text[startIdx-1]!='\t') &&
						(text[startIdx-1]!=',') &&
						(text[startIdx-1]!='<') && (text[startIdx-1]!='>') &&
						(text[startIdx-1]!='(') && (text[startIdx-1]!=')') &&
						(text[startIdx-1]!='[') && (text[startIdx-1]!=']') &&
						(text[startIdx-1]!='{') && (text[startIdx-1]!='}')
						)
					{
//						kdDebug(14010) << "searching start of email addy at: " << startIdx << endl;
						startIdx--;
					}
//					kdDebug(14010) << "found start of email addy at:" << startIdx << endl;

					regExp.setPattern( QString::fromLatin1( "[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if ( regExp.search(text,startIdx) != -1 )
					{
						matchLen = regExp.matchedLength();
						if (text[startIdx+matchLen-1]=='.')   // remove trailing dot
						{
							matchLen--;
						}
						else if (text[startIdx+matchLen-1]==',')   // remove trailing comma
						{
							matchLen--;
						}
						else if (text[startIdx+matchLen-1]==':')   // remove trailing colon
						{
							matchLen--;
						}

						if ( matchLen < 3 )
						{
							result += text[idx];
						}
						else
						{
//							kdDebug(14010) << "adding email link starting at: " << result.length()-(idx-startIdx) << endl;
							result.remove( result.length()-(idx-startIdx), idx-startIdx );
							QString mailAddr = parseHTML(text.mid(startIdx,matchLen),false);
							result += QString::fromLatin1("<a href=\"mailto:%1\">%2</a>").arg(mailAddr).arg(mailAddr);
/*								QString::fromLatin1("<a href=\"addrOrId://") + // What is this weird adress?
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("\">") +
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("</a>"); */
							idx = startIdx + matchLen - 1;
//							kdDebug(14010) << "index is now: " << idx << endl;
//							kdDebug(14010) << "result is: " << result << endl;
							lastReplacement = idx;
						}
						break;
					}
				}
				result += text[idx];
				break;
			}

			case 'h' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='t') )
				{   // don't do all the stuff for every 'h'
					regExp.setPattern( QString::fromLatin1( "https?://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if ( regExp.search(text,idx) == (int)idx )
					{
						matchLen = regExp.matchedLength();

						if (text[idx+matchLen-1]=='.')			// remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')		// remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')		// remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}

			case 'w':
			{
				if( (parseURLs) && (text[idx+1].latin1()=='w') && (text[idx+2].latin1()=='w') )
				{   // don't do all the stuff for every 'w'
					regExp.setPattern( QString::fromLatin1( "www\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if (regExp.search(text,idx)==(int)idx)
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"http://")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result+=text[idx];
				break;
			}

			case 'f' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='t') && (text[idx+2].latin1()=='p') )
				{   // don't do all the stuff for every 'f'
					regExp.setPattern( QString::fromLatin1( "ftp://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if ( regExp.search(text,idx)==(int)idx )
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
							matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
							matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
							matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}

					regExp.setPattern( QString::fromLatin1( "ftp\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if ( regExp.search(text,idx)==(int)idx )
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
						matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
						matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
						matchLen--;

						result +=
							QString::fromLatin1("<a href=\"ftp://")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result+=text[idx];
				break;
			}

			case 'm' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='a') && (text[idx+2].latin1()=='i') )
				{   // don't do all the stuff for every 'm'
					regExp.setPattern( QString::fromLatin1( "mailto:[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+" ) );
					if (regExp.search(text,idx)==(int)idx)
					{
						matchLen = regExp.matchedLength();
						if (text[idx+matchLen-1]=='.')   // remove trailing dot
						matchLen--;
						else if (text[idx+matchLen-1]==',')   // remove trailing comma
						matchLen--;
						else if (text[idx+matchLen-1]==':')   // remove trailing colon
						matchLen--;

						result +=
							QString::fromLatin1("<a href=\"")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("\">")
							+ text.mid(idx,matchLen)
							+ QString::fromLatin1("</a>");
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}
//TODO: Get a real RTF-Editor for Kopete and send html-ized texts out, this pseudo formatting
//      using ASCII-chars is ONLY common for UseNet.
//      And yes, I was the one who introduced it, now I think it's the wrong way ;) mETz [03.01.2003]

			case '_' :
			case '/' :
			case '*' :
			{
				regExp = QString::fromLatin1( "\\%1[^\\s%2]+\\%3" ).arg( text[ idx ] ).arg( text[ idx ] ).arg( text[ idx ] );
				if ( regExp.search(text,idx) == (int)idx )
				{
					matchLen = regExp.matchedLength();
					if ((matchLen>2) &&
					((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
					((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
					(text[idx+matchLen]=='.')||(text[idx+matchLen]==')')))
					{
						switch (text[idx].latin1())
						{
							case '_' :
								result += QString::fromLatin1( "<u>%1</u>" ).arg( parseHTML( text.mid( idx + 1, matchLen - 2 ), parseURLs ) );
								break;
							case '/' :
								result += QString::fromLatin1( "<i>%1</i>" ).arg( parseHTML( text.mid( idx + 1, matchLen - 2 ), parseURLs ) );
								break;
							case '*' :
								result += QString::fromLatin1( "<b>%1</b>" ).arg( parseHTML( text.mid( idx + 1, matchLen - 2 ), parseURLs ) );
								break;
						}
						idx += matchLen-1;
						lastReplacement = idx;
						break;
					}
				}
				result += text[idx];
				break;
			}

			default:
				result += text[idx];
				break;
		} // END switch( text[idx].latin1() )
	}
	return result;
}

QString KopeteMessage::asHTML() const
{
	QString msg = parsedBody();

	if ( fg().isValid() )
		msg.prepend( QString::fromLatin1( "<font color=\"%1\">" ).arg(fg().name()) );
	else
		msg.prepend( QString::fromLatin1( "<font>" ) );

	msg.append( QString::fromLatin1( "</font>" ) );

	// we want a custom background-color
	if ( bg().isValid() )
		msg.prepend( QString::fromLatin1( "<html><body bgcolor=\"%1\">" ).arg( bg().name() ) );
	else
		msg.prepend( QString::fromLatin1( "<html><body>" ) );

	msg.append ( QString::fromLatin1( "</body></html>" ) );
	return msg;
}

QDateTime KopeteMessage::timestamp() const
{
	return d->timeStamp;
}

const KopeteContact *KopeteMessage::from() const
{
	return d->from;
}

KopeteContactPtrList KopeteMessage::to() const
{
	return d->to;
}

KopeteMessage::MessageType KopeteMessage::type() const
{
	return d->type;
}

QColor KopeteMessage::fg() const
{
	return d->fgColor;
}

QColor KopeteMessage::bg() const
{
	return d->bgColor;
}

QFont KopeteMessage::font() const
{
	return d->font;
}

QString KopeteMessage::body() const
{
	return d->body;
}

QString KopeteMessage::subject() const
{
	return d->subject;
}

KopeteMessage::MessageFormat KopeteMessage::format() const
{
	return d->format;
}

KopeteMessage::MessageDirection KopeteMessage::direction() const
{
	return d->direction;
}

void KopeteMessage::detach()
{
	if( d->refCount == 1 )
		return;

	KopeteMessagePrivate *newD = new KopeteMessagePrivate;

	// Warning: this only works as long as the private object doesn't contain pointers to allocated objects.
	// The from contact for example is fine, but it's a shallow copy this way.
	*newD = *d;
	newD->refCount = 1;
	d->refCount--;

	d = newD;
}

// vim: set noet ts=4 sts=4 sw=4:

