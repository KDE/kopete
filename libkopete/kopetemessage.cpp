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

#include "kopetemessage.h"

#include "kopeteemoticons.h"



KopeteMessage::KopeteMessage()
{
	mTimestamp = QDateTime::currentDateTime();
	mBody = "Body not set";
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

	//TOTO: usescape and remove HTML
	kdDebug() << "KopeteMessage::plainBody: WARNING message non unescaped (TODO)" <<endl;
	return mBody;
}

QString KopeteMessage::escapedBody() const
{
				if(mFormat==PlainText)
				{
								QString parsedString = QStyleSheet::escape(mBody);
								kdDebug() << "KopeteMessage::escapeBody: " << parsedString <<endl;
								return parsedString;
				}

//	kdDebug() << "KopeteMessage::escapeBody: not escape needed" <<endl;
				return mBody;
}

QString KopeteMessage::parsedBody() const
{
	if(mFormat==ParsedHTML)
		return mBody;

	return KopeteEmoticons::parseEmoticons(parseHTML(escapedBody()));
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
//						kdDebug() << "searching start of email addy at: " << startIdx << endl;
						startIdx--;
					}

					kdDebug() << "found start of email addy at:" << startIdx << endl;

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
//							kdDebug() << "adding email link starting at: " << result.length()-(idx-startIdx) << endl;
							result.remove( result.length()-(idx-startIdx), idx-startIdx );
							QString mailAddr = parseHTML(text.mid(startIdx,matchLen),false);
							result += QString::fromLatin1("<a href=\"mailto:%1\">%2</a>").arg(mailAddr).arg(mailAddr);
/*								QString::fromLatin1("<a href=\"addrOrId://") + // What is this weird adress?
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("\">") +
								parseHTML(text.mid(startIdx,matchLen),false) +
								QString::fromLatin1("</a>"); */
							idx = startIdx + matchLen - 1;
							kdDebug() << "index is now: " << idx << endl;
							kdDebug() << "result is: " << result << endl;
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

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

