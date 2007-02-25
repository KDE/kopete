/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2006 by Olivier Goffart        <ogoffart@kde.org>

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
	QString body;
	QString subject;
};

Message::Private::Private( const QDateTime &timeStamp, const Contact *from,
		const ContactPtrList &to, const QString &subject,
		MessageDirection direction, const QString &requestedPlugin, MessageType type )
: from( const_cast<Contact*>(from) ), to( to ), manager( 0 ), direction( direction ), format( PlainText ), type( type ),
	requestedPlugin( requestedPlugin ), importance( (to.count() <= 1) ? Normal : Low ),
	bgOverride( false ), fgOverride( false ), rtfOverride( false ), isRightToLeft( false ),
	timeStamp( timeStamp ), body( QString::null ), subject( subject )
{
}

Message::Message()
: d( new Private( QDateTime::currentDateTime(), 0L, ContactPtrList(), QString(), Internal,
	QString(), TypeNormal ) )
{
}

Message::Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
: d( new Private( QDateTime::currentDateTime(), fromKC, toKC, QString::null, direction, requestedPlugin, type ) )
{
	doSetBody( body, f );
}

Message::Message( const Contact *fromKC, const Contact *toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
{
	QList<Contact*> to;
	to.append((Kopete::Contact*)toKC);
	d = new Private( QDateTime::currentDateTime(), fromKC, to, QString::null, direction, requestedPlugin, type );
	doSetBody( body, f );
}

Message::Message( const Contact *fromKC, const QList<Contact*> &toKC, const QString &body,
		  const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( QDateTime::currentDateTime(), fromKC, toKC, subject, direction, requestedPlugin, type ) )
{
	doSetBody( body, f );
}

Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		  const QString &body, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, QString::null, direction, requestedPlugin, type ) )
{
	doSetBody( body, f );
}


Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QList<Contact*> &toKC,
		  const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, subject, direction, requestedPlugin, type ) )
{
	doSetBody( body, f );
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

void Message::doSetBody( const QString &_body, Message::MessageFormat f )
{
	QString body = _body;
	//TODO: move that in ChatTextEditPart::contents
	if( f == RichText )
	{
		//This is coming from the RichTextEditor component.
		//Strip off the containing HTML document
		body.replace( QRegExp( QLatin1String(".*<body[^>]*>(.*)</body>.*") ), QLatin1String("\\1") );

		//Strip <p> tags
		body.replace( QLatin1String("<p>"), QString() );

		//Replace </p> with a <br/>
		body.replace( QLatin1String("</p>"), QLatin1String("<br/>") );

		//Remove trailing </br>
		if ( body.endsWith( QLatin1String("<br/>") ) )
			body.truncate( body.length() - 5 );
	
		body.remove( QLatin1String("\n") );
		body.replace( QRegExp( QLatin1String( "\\s\\s" ) ), QLatin1String( " &nbsp;" ) );
	}
	/*
	else if( f == ParsedHTML )
	{
		kWarning( 14000 ) << k_funcinfo << "using ParsedHTML which is internal !   message: " << body << kdBacktrace() << endl;
	}*/

	d->body = body;
	d->format = f;

	// unescaping is very expensive, do it only once and cache the result
	d->isRightToLeft = ( f & RichText ? unescape( d->body ).isRightToLeft() : d->body.isRightToLeft() );
}

void Message::setBody( const QString &body, MessageFormat f )
{
	doSetBody( body, f );
}

bool Message::isRightToLeft() const
{
	return d->isRightToLeft;
}

void Message::setImportance(Message::MessageImportance i)
{
	d->importance = i;
}

QString Message::unescape( const QString &xml )
{
	QString data = xml;

	// Remove linebreak and multiple spaces. First return nbsp's to normal spaces :)
	data = data.simplifyWhiteSpace();

	int pos;
	while ( ( pos = data.indexOf( '<' ) ) != -1 )
	{
		int endPos = data.find( '>', pos + 1 );
		if( endPos == -1 )
			break;    // No more complete elements left

		// Take the part between < and >, and extract the element name from that
		int matchWidth = endPos - pos + 1;
		QString match = data.mid( pos + 1, matchWidth - 2 ).simplifyWhiteSpace();
		int elemEndPos = match.indexOf( ' ' );
		QString elem = ( elemEndPos == -1 ? match.lower() : match.left( elemEndPos ).lower() );
		if ( elem == QLatin1String( "img" ) )
		{
			// Replace smileys with their original text'
			const QString attrTitle  = QLatin1String( "title=\"" );
			int titlePos    = match.find( attrTitle, elemEndPos );
			int titleEndPos = match.find( '"',       titlePos + attrTitle.length() );
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
	QString body=d->body;
	if( d->format & RichText )
	{
		body = unescape( body );
	}
	return body;
}

QString Message::escapedBody() const
{
	QString escapedBody=d->body;
//	kDebug(14000) << k_funcinfo << escapedBody << " " << d->rtfOverride << endl;

	if( d->format & PlainText )
	{
		escapedBody=escape( escapedBody );
	}
	else if( d->format & RichText && d->rtfOverride)
	{
		//remove the rich text
		escapedBody = escape (unescape( escapedBody ) );
	}

	return escapedBody;
}

QString Message::parsedBody() const
{
	//kDebug(14000) << k_funcinfo << "messageformat: " << d->format << endl;

	if( d->format == ParsedHTML )
	{
		return d->body;
	}
	else
	{
#ifdef __GNUC__
#warning Disable Emoticon parsing for now, it make QString cause a ASSERT error. (DarkShock)
#endif
#if 0
		return Kopete::Emoticons::parseEmoticons(parseLinks(escapedBody(), RichText));
#endif
		return escapedBody();
	}
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
	if ( format == ParsedHTML )
		return message;

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

QFont Message::font() const
{
	return d->font;
}

QString Message::subject() const
{
	return d->subject;
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
