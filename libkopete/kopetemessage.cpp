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
#include <qstylesheet.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "kopetemessage.h"
#include "kopeteemoticons.h"
#include "kopetemetacontact.h"
#include "kopeteprefs.h"

KopeteMessage::KopeteMessage()
{
	mTimestamp = QDateTime::currentDateTime();
	mBody = QString::null;
	mDirection = Internal;
	mBg = QColor();
	mFg = QColor();
	mFont = QFont();
	mFormat = PlainText;
}

KopeteMessage::KopeteMessage(const KopeteContact *fromKC,
		KopeteContactPtrList toKC, QString body, MessageDirection direction, MessageFormat f)
{
	init(QDateTime::currentDateTime(), fromKC, toKC, body, QString::null, direction, f);
}

KopeteMessage::KopeteMessage(const KopeteContact *fromKC,
		KopeteContactPtrList toKC, QString body, QString subject, MessageDirection direction, MessageFormat f)
{
	init(QDateTime::currentDateTime(), fromKC, toKC, body, subject, direction, f);
}

KopeteMessage::KopeteMessage(QDateTime timeStamp,
		const KopeteContact *fromKC, KopeteContactPtrList toKC, QString body,
		MessageDirection direction, MessageFormat f)
{
	init(timeStamp, fromKC, toKC, body, QString::null, direction, f);
}

KopeteMessage::KopeteMessage(QDateTime timeStamp,
		const KopeteContact *fromKC, KopeteContactPtrList toKC, QString body,
		QString subject, MessageDirection direction, MessageFormat f)
{
	init(timeStamp, fromKC, toKC, body, subject, direction, f);
}

void KopeteMessage::setBgOverride( bool enabled )
{
	mBgOverride = enabled;
}

void KopeteMessage::setFg(QColor color)
{
	mFg = color;
}

void KopeteMessage::setBg(QColor color)
{
	mBg = color;
}

void KopeteMessage::setFont(QFont font)
{
	mFont = font;
}

void KopeteMessage::setBody( const QString& body, MessageFormat f )
{
	if( mDirection == Outbound && body.startsWith( QString::fromLatin1( "/me " ) ) )
	{
		mBody = body.section( QString::fromLatin1( " " ), 1 ).prepend(
			QString::fromLatin1( " " ) ).prepend( mFrom->displayName() ).prepend( QString::fromLatin1( "*" ) );
	}
	else
	{
		mBody = body;
	}

	mFormat = f;
}

void KopeteMessage::init(QDateTime timeStamp, const KopeteContact * from,
		KopeteContactPtrList to, QString body, QString subject, MessageDirection direction, MessageFormat f)
{
	mTimestamp = timeStamp;
	mFrom = from;
	mTo = to;
	mSubject = subject;
	mDirection = direction;
	mFg = QColor();
	mBg = QColor();
	mFont = QFont();
	setBody( body, f );
	mBgOverride = false;
}


QString KopeteMessage::plainBody() const
{
	if(mFormat==PlainText)
		return mBody;

//	kdDebug(14010) << "KopeteMessage::plainBody: WARNING message non unescaped (TODO)" <<endl;

	//FIXME: is there a better way to unescape HTML?
	QString r=mBody;
	r = r.replace( QRegExp( QString::fromLatin1( "<br>" ) ), QString::fromLatin1( "\n" ) ).
		replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::fromLatin1( "" ) ).
		replace( QRegExp( QString::fromLatin1( "&gt;" ) ), QString::fromLatin1( ">" ) ).
		replace( QRegExp( QString::fromLatin1( "&lt;" ) ), QString::fromLatin1( "<" ) ).
		replace( QRegExp( QString::fromLatin1( "&nbsp;" ) ), QString::fromLatin1( " " ) ).
		replace( QRegExp( QString::fromLatin1( "&amp;" ) ), QString::fromLatin1( "&" ) );

	kdDebug(14010) << "KopeteMessage::plainBody: " << r <<endl;
	return r;
}

QString KopeteMessage::escapedBody() const
{
	if( mFormat == PlainText )
	{
		QString parsedString = QStyleSheet::escape( mBody ).replace( QRegExp( QString::fromLatin1( "\n" ) ), QString::fromLatin1( "<br>\n" ) );
		// Replace multiple spaces with '&nbsp;', but leave the first space
		// intact for any necessary wordwrap:
		QStringList words = QStringList::split( ' ', parsedString, true );
		parsedString = "";
		for( QStringList::Iterator it = words.begin(); it != words.end(); ++it )
		{
			if( ( *it ).isEmpty() )
				parsedString += QString::fromLatin1( "&nbsp;" );
			else
				parsedString += *it + QString::fromLatin1( " " );
		}

		// Lastly, remove trailing whitespace:
		parsedString.replace( QRegExp( QString::fromLatin1( "\\s*$" ) ), QString::null );

		kdDebug(14010) << "KopeteMessage::escapeBody: " << parsedString <<endl;
		return parsedString;
	}

	kdDebug(14010) << "KopeteMessage::escapeBody: not escape needed" <<endl;
	return mBody;
}

QString KopeteMessage::parsedBody() const
{
	if(mFormat==ParsedHTML)
		return mBody;

	return KopeteEmoticons::parseEmoticons(parseHTML(escapedBody()));
}

QString KopeteMessage::transformMessage( const QString &model ) const
{
	QString message;
	bool F_first = true;
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
					message.append( KGlobal::locale()->formatTime(mTimestamp.time(), true) );
					break;

				case 'F':  //insert Fonts
					if( F_first ) // <font>....
					{
						message += QString::fromLatin1( "<font " );
						if ( mFg.isValid() )
							message += QString::fromLatin1( "color=\"" ) + mFg.name() + QString::fromLatin1( "\"" );
						if ( mFont != QFont() )
							message += QString::fromLatin1( " face=\"" ) + mFont.family() + QString::fromLatin1( "\"" );
						message += QString::fromLatin1( ">" );
						if ( mFont != QFont() && mFont.bold())
							message += QString::fromLatin1( "<b>" );
						if ( mFont != QFont() && mFont.italic())
							message += QString::fromLatin1( "<i>" );
						F_first=false;
					}
					else            // </font>
					{
						if ( mFont != QFont() && mFont.italic())
							message += QString::fromLatin1( "</i>" );
						if ( mFont != QFont() && mFont.bold())
							message += QString::fromLatin1( "</b>" );

						message += QString::fromLatin1( "</font>" );
						F_first=true;
					}
					break;

				case 'b':   //BgColor
					if ( mBg.isValid() && !mBgOverride )
						message += QString::fromLatin1( "bgcolor=\"" ) + mBg.name() + QString::fromLatin1( "\"" );
					break;

				case 'i': //only inbound
					if(mDirection != Inbound)
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
					if(mDirection != Outbound)
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
					if(mDirection != Internal)
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
				case 'e': //not internal (external)
					if(mDirection == Internal)
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

				case 'f': //insert the 'from' displayName
					if (mFrom->metaContact())
						message.append( QStyleSheet::escape(mFrom->metaContact()->displayName()) );
					else
						message.append( QStyleSheet::escape(mFrom->displayName()) );
					break;

				case 't': //insert the 'to' displayName
					if (to().first()->metaContact())
						message.append( QStyleSheet::escape(to().first()->metaContact()->displayName()) );
					else
						message.append( QStyleSheet::escape(to().first()->displayName()) );
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

QString KopeteMessage::parseHTML( QString message, bool parseURLs )
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
			case '\n':
				lastReplacement = idx;
				result += QString::fromLatin1( "<br>" );
				break;
			case '\t':		// tab == 4 spaces
				lastReplacement = idx;
				result += QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" );
				break;
			case ' ':		// convert doubles spaces to HTML
			{
				if( (idx>0) && (text[idx-1]==' '))
					result += QString::fromLatin1( "&nbsp;" );
				else
					result += QString::fromLatin1( " " );
				lastReplacement = idx;
				break;
			}

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
		msg.prepend( QString::fromLatin1( "<FONT COLOR=\"%1\">" ).arg(fg().name()) );
	else
		msg.prepend( QString::fromLatin1( "<FONT>" ) );

	msg.append( QString::fromLatin1( "</FONT>" ) );

	// we want a custom background-color
	if ( bg().isValid() )
		msg.prepend( QString::fromLatin1( "<HTML><BODY BGCOLOR=\"%1\">" ).arg( bg().name() ) );
	else
		msg.prepend( QString::fromLatin1( "<HTML><BODY>" ) );

	msg.append ( QString::fromLatin1( "</BODY></HTML>" ) );
	return msg;
}

// vim: set noet ts=4 sts=4 sw=4:

