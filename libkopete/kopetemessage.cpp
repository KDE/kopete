/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2006 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2006-2007 by Charles Connell        <charles@connells.org>

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

#include <stdlib.h>

#include <qcolor.h>
#include <qbuffer.h>
#include <qimage.h>
#include <QTextDocument>
#include <qregexp.h>
#include <qtextcodec.h>
#include <QByteArray>
#include <QSharedData>
#include <QPointer>
#include <QtCore/QLatin1String>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kcodecs.h>

#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopeteemoticons.h"


namespace Kopete
{

class Message::Private
	: public QSharedData
{
public:
	Private( const QDateTime &timeStamp, const Contact *from, const ContactPtrList &to,
	         const QString &subject, MessageDirection direction,
	         const QString &requestedPlugin, MessageType type );
	Private (const Private &other);
	~Private ();

	QPointer<Contact> from;
	ContactPtrList to;
	ChatSession *manager;

	MessageDirection direction;
	MessageFormat format;
	MessageType type;
	QString requestedPlugin;
	MessageImportance importance;
	bool bgOverride;
	bool fgOverride;
	bool rtfOverride;
	bool isRightToLeft;
	QDateTime timeStamp;
	QFont font;
	QStringList classes;

	QColor fgColor;
	QColor bgColor;
	QString subject;

	QTextDocument* body;
	mutable QString escapedBody;
	mutable bool escapedBodyDirty;
};

Message::Private::Private( const QDateTime &timeStamp, const Contact *from,
		const ContactPtrList &to, const QString &subject,
		MessageDirection direction, const QString &requestedPlugin, MessageType type )
	: from( const_cast<Contact*>(from) ), to(to), manager(0), direction(direction), format(PlainText), type(type)
	, requestedPlugin(requestedPlugin), importance( (to.count() <= 1) ? Normal : Low ), bgOverride(false), fgOverride(false)
	, rtfOverride(false), isRightToLeft (false), timeStamp(timeStamp), subject(subject), body(new QTextDocument()), escapedBodyDirty(true)
{
}

Message::Private::Private (const Message::Private &other)
	: QSharedData (other)
{
	from = other.from;
	to = other.to;
	manager = other.manager;

	direction = other.direction;
	format = other.format;
	type = other.type;
	requestedPlugin = other.requestedPlugin;
	importance = other.importance;
	bgOverride = other.bgOverride;
	fgOverride = other.fgOverride;
	rtfOverride = other.rtfOverride;
	isRightToLeft = other.isRightToLeft;
	timeStamp = other.timeStamp;
	font = other.font;
	classes = other.classes;

	fgColor = other.fgColor;
	bgColor = other.bgColor;
	subject = other.subject;

	body = other.body->clone();
	escapedBody = other.escapedBody;
	escapedBodyDirty = other.escapedBodyDirty;
}

Message::Private::~Private ()
{
	delete body;
}

Message::Message()
    : d( new Private( QDateTime::currentDateTime(), 0L, ContactPtrList(), QString(), Internal,
	QString(), TypeNormal ) )
{
}

Message::Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( QDateTime::currentDateTime(), fromKC, toKC, QString(), direction, requestedPlugin, type ) )
{
	doSetBody(body, f);
}

Message::Message( const Contact *fromKC, const Contact *toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
{
	QList<Contact*> to;
	to.append((Kopete::Contact*)toKC);
	d = new Private( QDateTime::currentDateTime(), fromKC, to, QString(), direction, requestedPlugin, type );
	doSetBody(body, f);
}

Message::Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		  const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( QDateTime::currentDateTime(), fromKC, toKC, subject, direction, requestedPlugin, type ) )
{
	doSetBody(body, f);
}

Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		  const QString &body, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, QString(), direction, requestedPlugin, type ) )
{
	doSetBody(body, f);
}


Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		  const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, subject, direction, requestedPlugin, type ) )
{
	doSetBody(body, f);
}

Message::Message( const Message &other )
	: d(other.d)
{
}

Message& Message::operator=( const Message &other )
{
	d = other.d;
	return *this;
}

Message::~Message()
{
}

void Message::setBgOverride( bool enabled )
{
	d->bgOverride = enabled;
}

void Message::setFgOverride( bool enabled )
{
	d->fgOverride = enabled;
}

void Message::setRtfOverride( bool enabled )
{
	d->rtfOverride = enabled;
}

void Message::setFg( const QColor &color )
{
	d->fgColor=color;
}

void Message::setBg( const QColor &color )
{
	d->bgColor=color;
}

void Message::setFont( const QFont &font )
{
	d->font = font;
}

void Message::setBody( const QString &body, MessageFormat f )
{
	doSetBody (body, f);
}

void Message::setPlainBody (const QString &body)
{
	doSetBody (body, PlainText);
}

void Message::setHtmlBody (const QString &body)
{
	doSetBody (body, RichText);
}

void Message::doSetBody (const QString &body, MessageFormat f)
{
	if (f == PlainText)
		d->body->setPlainText(body);
	else
		d->body->setHtml(body);
	d->format = f;
	d->isRightToLeft = d->body->toPlainText().isRightToLeft();
	d->escapedBodyDirty = true;
}

void Message::setBody (const QTextDocument *_body)
{
	doSetBody (_body, RichText);
}

void Message::doSetBody (const QTextDocument *body, MessageFormat f)
{
	delete d->body;
	d->body = body->clone();          // delete the old body and replace it with a *copy* of the new one
	d->format = f;
	d->isRightToLeft = d->body->toPlainText().isRightToLeft();
	d->escapedBodyDirty = true;
}

void Message::setImportance(Message::MessageImportance i)
{
	d->importance = i;
}

QString Message::unescape( const QString &xml )
{
	QString data = xml;

	// Remove linebreak and multiple spaces. First return nbsp's to normal spaces :)
	data = data.simplified();

	int pos;
	while ( ( pos = data.indexOf( '<' ) ) != -1 )
	{
		int endPos = data.indexOf( '>', pos + 1 );
		if( endPos == -1 )
			break;    // No more complete elements left

		// Take the part between < and >, and extract the element name from that
		int matchWidth = endPos - pos + 1;
		QString match = data.mid( pos + 1, matchWidth - 2 ).simplified();
		int elemEndPos = match.indexOf( ' ' );
		QString elem = ( elemEndPos == -1 ? match.toLower() : match.left( elemEndPos ).toLower() );
		if ( elem == QLatin1String( "img" ) )
		{
			// Replace smileys with their original text'
			const QString attrTitle  = QLatin1String( "title=\"" );
			int titlePos    = match.indexOf( attrTitle, elemEndPos );
			int titleEndPos = match.indexOf( '"',       titlePos + attrTitle.length() );
			if( titlePos == -1 || titleEndPos == -1 )
			{
				// Not a smiley but a normal <img>
				// Don't update pos, we restart at this position :)
				data.remove( pos, matchWidth );
			}
			else
			{
				QString orig = match.mid( titlePos + attrTitle.length(),
					titleEndPos - titlePos - attrTitle.length() );
				data.replace( pos, matchWidth, orig );
				pos += orig.length();
			}
		}
		else if ( elem == QLatin1String( "/p" ) || elem == QLatin1String( "/div" ) ||
			elem == QLatin1String( "br" ) )
		{
			// Replace paragraph, div and line breaks with a newline
			data.replace( pos, matchWidth, '\n' );
			pos++;
		}
		else
		{
			// Remove all other elements entirely
			// Don't update pos, we restart at this position :)
			data.remove( pos, matchWidth );
		}
	}

	// Replace stuff starting with '&'
	data.replace( QLatin1String( "&gt;" ), QLatin1String( ">" ) );
	data.replace( QLatin1String( "&lt;" ), QLatin1String( "<" ) );
	data.replace( QLatin1String( "&quot;" ), QLatin1String( "\"" ) );
	data.replace( QLatin1String( "&nbsp;" ), QLatin1String( " " ) );
	data.replace( QLatin1String( "&amp;" ), QLatin1String( "&" ) );
	data.replace( QLatin1String( "&#160;" ), QLatin1String( " " ) );  //this one is used in jabber:  note, we should escape all &#xx;

	return data;
}

QString Message::escape( const QString &text )
{
	QString html = Qt::escape( text );
 	//Replace carriage returns inside the text
	html.replace( QLatin1String( "\n" ), QLatin1String( "<br />" ) );
	//Replace a tab with 4 spaces
	html.replace( QLatin1String( "\t" ), QLatin1String( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

	//Replace multiple spaces with &nbsp;
	//do not replace every space so we break the linebreak
	html.replace( QRegExp( QLatin1String( "\\s\\s" ) ), QLatin1String( "&nbsp; " ) );

	return html;
}

QString Message::plainBody() const
{
	return d->body->toPlainText();
}

QString Message::escapedBody() const
{
//	kDebug(14010) << k_funcinfo << escapedBody() << " " << d->rtfOverride << endl;

//	the escaped body is cached because QRegExp is very expensive, so it shouldn't be used any more than nescessary
	if (!d->escapedBodyDirty)
		return d->escapedBody;
	else {
		QString html = d->body->toHtml();
//		all this regex business is to take off the outer HTML document provided by QTextDocument
//		remove the head
		QRegExp badStuff ("<head[^<>]*>.*</head[^<>]*>");
		html = html.remove (badStuff);
//		remove the <html> and </html> tags
		badStuff.setPattern ("</?html[^<>]*>");
		html = html.remove (badStuff);
//		remove the <body> and </body> tags
		badStuff.setPattern ("</?body[^<>]*>");
		html = html.remove (badStuff);
//		remove newlines that may be present, since they end up being displayed in the chat window. real newlines are represented with <br>, so we know \n's are meaningless (I hope this is true, could anybody confirm? (C Connell))
		html.remove ("\n");
		d->escapedBody = html;
		d->escapedBodyDirty = false;
		return html;
	}
}

QString Message::parsedBody() const
{
	//kDebug(14000) << k_funcinfo << "messageformat: " << d->format << endl;
#ifdef __GNUC__
#warning Disable Emoticon parsing for now, it make QString cause a ASSERT error. (DarkShock)
#endif
#if 0
	return Kopete::Emoticons::parseEmoticons(parseLinks(escapedBody(), RichText));
#endif
	return escapedBody();
}

static QString makeRegExp( const char *pattern )
{
	const QString urlChar = QLatin1String( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString boundaryStart = QString( "(^|[^%1])(" ).arg( urlChar );
	const QString boundaryEnd = QString( ")([^%1]|$)" ).arg( urlChar );

	return boundaryStart + QLatin1String(pattern) + boundaryEnd;
}

QString Message::parseLinks( const QString &message, MessageFormat format )
{
	if ( format & RichText )
	{
		// < in HTML *always* means start-of-tag
		QStringList entries = message.split( QChar('<'), QString::KeepEmptyParts );

		QStringList::Iterator it = entries.begin();

		// first one is different: it doesn't start with an HTML tag.
		if ( it != entries.end() )
		{
			*it = parseLinks( *it, PlainText );
			++it;
		}

		for ( ; it != entries.end(); ++it )
		{
			QString curr = *it;
			// > in HTML means start-of-tag if and only if it's the first one after a <
			int tagclose = curr.indexOf( QChar('>') );
			// no >: the HTML is broken, but we can cope
			if ( tagclose == -1 )
				continue;
			QString tag = curr.left( tagclose + 1 );
			QString body = curr.mid( tagclose + 1 );
			*it = tag + parseLinks( body, PlainText );
		}
		return entries.join(QLatin1String("<"));
	}

	QString result = message;

	// common subpatterns - may not contain matching parens!
	const QString name = QLatin1String( "[\\w\\+\\-=_\\.]+" );
	const QString userAndPassword = QString( "(?:%1(?::%1)?\\@)" ).arg( name );
	const QString urlChar = QLatin1String( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString urlSection = QString( "[%1]+" ).arg( urlChar );
	const QString domain = QLatin1String( "[\\-\\w_]+(?:\\.[\\-\\w_]+)+" );

	//Replace http/https/ftp links:
	// Replace (stuff)://[user:password@](linkstuff) with a link
	result.replace(
		QRegExp( makeRegExp("\\w+://%1?\\w%2").arg( userAndPassword, urlSection ) ),
		QLatin1String("\\1<a href=\"\\2\" title=\"\\2\">\\2</a>\\3" ) );

	// Replace www.X.Y(linkstuff) with a http: link
	result.replace(
		QRegExp( makeRegExp("%1?www\\.%2%3").arg( userAndPassword, domain, urlSection ) ),
		QLatin1String("\\1<a href=\"http://\\2\" title=\"http://\\2\">\\2</a>\\3" ) );

	//Replace Email Links
	// Replace user@domain with a mailto: link
	result.replace(
		QRegExp( makeRegExp("%1@%2").arg( name, domain ) ),
		QLatin1String("\\1<a href=\"mailto:\\2\" title=\"mailto:\\2\">\\2</a>\\3") );

	//Workaround for Bug 85061: Highlighted URLs adds a ' ' after the URL itself
	// the trailing  &nbsp; is included in the url.
	result.replace( QRegExp( QLatin1String("(<a href=\"[^\"]+)(&nbsp;)(\")")  ) , QLatin1String("\\1\\3") );

	return result;
}



QDateTime Message::timestamp() const
{
	return d->timeStamp;
}

const Contact *Message::from() const
{
	return d->from;
}

QList<Contact*> Message::to() const
{
	return d->to;
}

Message::MessageType Message::type() const
{
	return d->type;
}

QString Message::requestedPlugin() const
{
	return d->requestedPlugin;
}

QColor Message::fg() const
{
	return d->fgColor;
}

QColor Message::bg() const
{
	return d->bgColor;
}

bool Message::isRightToLeft() const
{
	return d->isRightToLeft;
}
QFont Message::font() const
{
	return d->font;
}

QString Message::subject() const
{
	return d->subject;
}

const QTextDocument *Message::body() const
{
	return d->body;
}

Message::MessageFormat Message::format() const
{
	return d->format;
}

Message::MessageDirection Message::direction() const
{
	return d->direction;
}

Message::MessageImportance Message::importance() const
{
	return d->importance;
}

ChatSession *Message::manager() const
{
	return d->manager;
}

void Message::setManager(ChatSession *kmm)
{
	d->manager=kmm;
}

QString Message::getHtmlStyleAttribute() const
{
	QString styleAttribute;
	
	styleAttribute = QString::fromUtf8("style=\"");

	// Affect foreground(color) and background color to message.
	if( !d->fgOverride && d->fgColor.isValid() )
	{
		styleAttribute += QString::fromUtf8("color: %1; ").arg(d->fgColor.name());
	}
	if( !d->bgOverride && d->bgColor.isValid() )
	{
		styleAttribute += QString::fromUtf8("background-color: %1; ").arg(d->bgColor.name());
	}
	
	// Affect font parameters.
	if( !d->rtfOverride && d->font!=QFont() )
	{
		QString fontstr;
		if(!d->font.family().isNull())
			fontstr+=QLatin1String("font-family: ")+d->font.family()+QLatin1String("; ");
		if(d->font.italic())
			fontstr+=QLatin1String("font-style: italic; ");
		if(d->font.strikeOut())
			fontstr+=QLatin1String("text-decoration: line-through; ");
		if(d->font.underline())
			fontstr+=QLatin1String("text-decoration: underline; ");
		if(d->font.bold())
			fontstr+=QLatin1String("font-weight: bold;");

		styleAttribute += fontstr;
	}

	styleAttribute += QString::fromUtf8("\"");

	return styleAttribute;
}

QString Message::decodeString( const QByteArray &message, const QTextCodec *providedCodec, bool *success )
{
	/*
	Note to everyone. This function is not the most efficient, that is for sure.
	However, it *is* the only way we can be guaranteed that a given string is
	decoded properly.
	*/

	if( success )
		*success = true;

	// Avoid heavy codec tests on empty message.
	if( message.isEmpty() )
            return QString::fromAscii( message );

	//Check first 128 chars
	int charsToCheck = message.length();
	charsToCheck = 128 > charsToCheck ? charsToCheck : 128;

#ifdef __GNUC__
	#warning Rewrite the following code: heuristicContentMatch() do not existe anymore.
#endif
	//They are providing a possible codec. Check if it is valid
//	if( providedCodec && providedCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		return providedCodec->toUnicode( message );
	}

	//Check if it is UTF
	if( KStringHandler::isUtf8(message) )
	{
		//We have a UTF string almost for sure. At least we know it will be decoded.
		return QString::fromUtf8( message );
	}
/*
	//Try codecForContent - exact match
	QTextCodec *testCodec = QTextCodec::codecForContent(message, charsToCheck);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		return testCodec->toUnicode( message );
	}

	kWarning(14000) << k_funcinfo << "Unable to decode string using provided codec(s), taking best guesses!" << endl;
	if( success )
		*success = false;

	//We don't have any clues here.

	//Try local codec
	testCodec = QTextCodec::codecForLocale();
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kDebug(14000) << k_funcinfo << "Using locale's codec" << endl;
		return testCodec->toUnicode( message );
	}

	//Try latin1 codec
	testCodec = QTextCodec::codecForMib(4);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kDebug(14000) << k_funcinfo << "Using latin1" << endl;
		return testCodec->toUnicode( message );
	}

	kDebug(14000) << k_funcinfo << "Using latin1 and cleaning string" << endl;
	//No codec decoded. Just decode latin1, and clean out any junk.
	QString result = QLatin1String( message );
	const uint length = message.length();
	for( uint i = 0; i < length; ++i )
	{
		if( !result[i].isPrint() )
			result[i] = '?';
	}

	return result;
*/
	return QString();
}

QStringList Message::classes() const
{
	return d->classes;
}

void Message::addClass(const QString & classe)
{
	d->classes.append(classe);
}

void Message::setClasses(const QStringList & classes)
{
	d->classes = classes;
}

}
