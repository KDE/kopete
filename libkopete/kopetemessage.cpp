/*
	kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

void KopeteMessage::setBody( const QString& body , MessageFormat f )
{
	mBody = body;
	mFormat = f;
}

void KopeteMessage::init(QDateTime timeStamp, const KopeteContact * from,
		KopeteContactPtrList to, QString body, QString subject, MessageDirection direction, MessageFormat f)
{
	mTimestamp = timeStamp;
	mFrom = from;
	mTo = to;
	mBody = body;
	mSubject = subject;
	mDirection = direction;
	mFg = QColor();
	mBg = QColor();
	mFont = QFont();
	mFormat = f;
}


QString KopeteMessage::plainBody() const
{
	if(mFormat==PlainText)
		return mBody;

//	kdDebug(14010) << "KopeteMessage::plainBody: WARNING message non unescaped (TODO)" <<endl;

	//FIXME: is there a better way to unescape HTML?
	QString r=mBody;
	r = r.replace(QRegExp("<br>"), "\n").
		replace(QRegExp("<[^>]*>"), "").
		replace(QRegExp("&gt;"), ">").
		replace(QRegExp("&lt;"), "<").
		replace(QRegExp("&nbsp;"), " ").
		replace(QRegExp("&amp;"), "&");

	kdDebug(14010) << "KopeteMessage::plainBody: " << r <<endl;
	return r;
}

QString KopeteMessage::escapedBody() const
{
	if( mFormat == PlainText )
	{
		QString parsedString = QStyleSheet::escape( mBody ).replace( QRegExp( "\n" ), "<br>\n" );
		// Replace multiple spaces with '&nbsp;', but leave the first space
		// intact for any necessary wordwrap:
		QStringList words = QStringList::split( ' ', parsedString, true );
		parsedString = "";
		for( QStringList::Iterator it = words.begin(); it != words.end(); ++it )
		{
			if( ( *it ).isEmpty() )
				parsedString += "&nbsp;";
			else
				parsedString += *it + " ";
		}

		// Lastly, remove trailing whitespace:
		parsedString.replace( QRegExp( "\\s*$" ), "" );

		kdDebug(14010) << "KopeteMessage::escapeBody: " << parsedString <<endl;
		return parsedString;
	}

//	kdDebug(14010) << "KopeteMessage::escapeBody: not escape needed" <<endl;
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
	QString message = "";
	bool F_first = true;
	unsigned int f = 0;

	do
	{
		char c = ((const char*)model)[f];
		if( c != '%' )
		{
			message += c;
		}
		else
		{
			f++;
			c = ((const char*)model)[f];
			switch(c)
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
						message +="<font ";
						if ( mFg.isValid() )
							message += "color=\"" + mFg.name() + "\"";
						if ( mFont != QFont() )
							message += " face=\"" + mFont.family() + "\"";
						message +=">";
						if ( mFont != QFont() && mFont.bold())
							message += "<b>";
						if ( mFont != QFont() && mFont.italic())
							message += "<i>";
						F_first=false;
					}
					else            // </font>
					{
						if ( mFont != QFont() && mFont.italic())
							message += "</i>";
						if ( mFont != QFont() && mFont.bold())
							message += "</b>";

						message +="</font>";
						F_first=true;
					}
					break;

				case 'b':   //BgColor
					if ( mBg.isValid() )
						message += "bgcolor=\"" + mBg.name() + "\"";
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
							char c2=((const char*)model)[f];
							if(c2=='i' && b)
								fin=true;
							b = (c2=='%');
						}
						while (f<model.length() && !fin);
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
							char c2=((const char*)model)[f];
							if(c2=='o' && b)
								fin=true;
							b=(c2=='%');
						}
						while (f<model.length() && !fin);
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
							char c2=((const char*)model)[f];
							if(c2=='s' && b)
								fin=true;
							b=(c2=='%');
						}
						while (f<model.length() && !fin);
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
							char c2=((const char*)model)[f];
							if(c2=='e' && b)
								fin=true;
							b=(c2=='%');
						}
						while (f<model.length() && !fin);
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
				result += "<br>";
				break;
			case '\t':		// tab == 4 spaces
				lastReplacement = idx;
				result += "&nbsp;&nbsp;&nbsp;&nbsp;";
				break;
			case ' ':		// convert doubles spaces to HTML
				if( (idx>0) && (text[idx-1]==' '))
					result += "&nbsp;";
				else
					result += " ";
				lastReplacement = idx;
				break;

			case '@':		// email-addresses or message-ids
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

					kdDebug(14010) << "found start of email addy at:" << startIdx << endl;

					regExp.setPattern("[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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
							kdDebug(14010) << "index is now: " << idx << endl;
							kdDebug(14010) << "result is: " << result << endl;
							lastReplacement = idx;
						}
						break;
					}
				}
				result += text[idx];
				break;

			case 'h' :
			{
				if( (parseURLs) && (text[idx+1].latin1()=='t') )
				{   // don't do all the stuff for every 'h'
					regExp.setPattern("https?://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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
					regExp.setPattern("www\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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
					regExp.setPattern("ftp://[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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

					regExp.setPattern("ftp\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+\\.[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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
					regExp.setPattern("mailto:[^\\s<>\\(\\)\"\\|\\[\\]\\{\\}]+");
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

			case '_' :
			case '/' :
			case '*' :
			{
				regExp = QString("\\%1[^\\s%2]+\\%3").arg(text[idx]).arg(text[idx]).arg(text[idx]);
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
								result += QString("<u>%1</u>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
								break;
							case '/' :
								result += QString("<i>%1</i>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
								break;
							case '*' :
								result += QString("<b>%1</b>").arg( parseHTML(text.mid(idx+1,matchLen-2),parseURLs) );
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
		}
	}
	return result;
}

QString KopeteMessage::asHTML() const
{
	QString msg = parsedBody();

	if ( fg().isValid() )
		msg.prepend( QString("<FONT COLOR=\"%1\">").arg(fg().name()) );
	else
		msg.prepend( QString("<FONT>") );

	msg.append ( "</FONT>" );

	// we want a custom background-color
	if ( bg().isValid() )
		msg.prepend( QString("<HTML><BODY BGCOLOR=\"%1\">").arg(bg().name()) );
	else
		msg.prepend ( QString("<HTML><BODY>") );

	msg.append ( "</BODY></HTML>" );
	return msg;
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

