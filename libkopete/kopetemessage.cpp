/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
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

#include <stdlib.h>

#include <qcolor.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qstylesheet.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kmdcodec.h>

#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetechatsession.h"
#include "kopeteprefs.h"
#include "kopetecontact.h"
#include "kopeteemoticons.h"


using namespace Kopete;

class Message::Private
	: public KShared
{
public:
	Private( const QDateTime &timeStamp, const Contact *from, const ContactPtrList &to,
	         const QString &body, const QString &subject, MessageDirection direction, MessageFormat f,
	         const QString &requestedPlugin, MessageType type );

	const Contact *from;
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
	QDateTime timeStamp;
	QFont font;

	QColor fgColor;
	QColor bgColor;
	QString body;
	QString subject;
};

Message::Private::Private( const QDateTime &timeStamp, const Contact *from,
             const ContactPtrList &to, const QString &body, const QString &subject,
				 MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
	: from(from), to(to), manager(0), direction(direction), format(f), type(type)
	, requestedPlugin(requestedPlugin), importance( (to.count() <= 1) ? Normal : Low ), bgOverride(false), fgOverride(false)
	, rtfOverride(false), timeStamp(timeStamp), body(body), subject(subject)
{
	if( format == RichText )
	{
		//This is coming from the RichTextEditor component.
		//Strip off the containing HTML document
		this->body.replace( QRegExp( QString::fromLatin1(".*<body.*>\\s+(.*)\\s+</body>.*") ), QString::fromLatin1("\\1") );

		//Strip <p> tags
		this->body.replace( QString::fromLatin1("<p>"), QString::null );

		//Replace </p> with a <br/>
		this->body.replace( QString::fromLatin1("</p>") , QString::fromLatin1("<br/>") );

		//Remove trailing <br/>
		if ( this->body.endsWith( QString::fromLatin1("<br/>") ) )
			this->body.truncate( this->body.length() - 5 );
		this->body.remove(  QString::fromLatin1("\n") );
	}
}


Message::Message()
    : d( new Private( QDateTime::currentDateTime(), 0L, QPtrList<Contact>(), QString::null, QString::null, Internal,
	PlainText, QString::null, TypeNormal ) )
{
}

Message::Message( const Contact *fromKC, const QPtrList<Contact> &toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( QDateTime::currentDateTime(), fromKC, toKC, body, QString::null, direction, f, requestedPlugin, type ) )
{
}

Message::Message( const Contact *fromKC, const Contact *toKC, const QString &body,
		  MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
{
	QPtrList<Contact> to;
	to.append(toKC);
	d = new Private( QDateTime::currentDateTime(), fromKC, to, body, QString::null, direction, f, requestedPlugin, type );
}

Message::Message( const Contact *fromKC, const QPtrList<Contact> &toKC, const QString &body,
		  const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( QDateTime::currentDateTime(), fromKC, toKC, body, subject, direction, f, requestedPlugin, type ) )
{
}

Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QPtrList<Contact> &toKC,
		  const QString &body, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, body, QString::null, direction, f, requestedPlugin, type ) )
{
}


Message::Message( const QDateTime &timeStamp, const Contact *fromKC, const QPtrList<Contact> &toKC,
		  const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, const QString &requestedPlugin, MessageType type )
    : d( new Private( timeStamp, fromKC, toKC, body, subject, direction, f, requestedPlugin, type ) )
{
}

Kopete::Message::Message( const Message &other )
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

void Message::detach()
{
	// there is no detach in KSharedPtr :(
	if( d.count() == 1 )
		return;

	// Warning: this only works as long as the private object doesn't contain pointers to allocated objects.
	// The from contact for example is fine, but it's a shallow copy this way.
	d = new Private(*d);
}

void Message::setBgOverride( bool enabled )
{
	detach();
	d->bgOverride = enabled;
}

void Message::setFgOverride( bool enabled )
{
	detach();
	d->fgOverride = enabled;
}

void Message::setRtfOverride( bool enabled )
{
	detach();
	d->rtfOverride = enabled;
}

void Message::setFg( const QColor &color )
{
	detach();
	d->fgColor=color;
}

void Message::setBg( const QColor &color )
{
	detach();
	d->bgColor=color;
}

void Message::setFont( const QFont &font )
{
	detach();
	d->font = font;
}

void Message::setBody( const QString &body, MessageFormat f )
{
	detach();

	QString theBody = body;
	if( f == RichText )
	{
		//This is coming from the RichTextEditor component.
		//Strip off the containing HTML document
		theBody.replace( QRegExp( QString::fromLatin1(".*<body.*>\\s+(.*)\\s+</body>.*") ), QString::fromLatin1("\\1") );

		//Strip <p> tags
		theBody.replace( QString::fromLatin1("<p>"), QString::null );

		//Replace </p> with a <br/>
		theBody.replace( QString::fromLatin1("</p>"), QString::fromLatin1("<br/>") );

		//Remove trailing </br>
		if ( theBody.endsWith( QString::fromLatin1("<br/>") ) )
			theBody.truncate( theBody.length() - 5 );

		theBody.remove( QString::fromLatin1("\n") );
	}

	d->body=body;
	d->format = f;
}

void Message::setImportance(Message::MessageImportance i)
{
	detach();
	d->importance = i;
}

QString Message::unescape( const QString &xml )
{
	QString data = xml;

	//remove linebreak and multiple spaces
	data.replace( QRegExp( QString::fromLatin1( "\\s*[\n\r\t]+\\s*" ) , false ), QString::fromLatin1(" " )) ;

	data.replace( QRegExp( QString::fromLatin1( "< *img[^>]*title=\"([^>\"]*)\"[^>]*>" ) , false ), QString::fromLatin1( "\\1" ) );  //escape smeleys, return to the original code
	data.replace( QRegExp( QString::fromLatin1( "< */ *p[^>]*>" ) , false ), QString::fromLatin1( "\n" ) );
	data.replace( QRegExp( QString::fromLatin1( "< */ *div[^>]*>" ) , false ), QString::fromLatin1( "\n" ) );
	data.replace( QRegExp( QString::fromLatin1( "< *br */? *>" ) , false ), QString::fromLatin1( "\n" ) );
	data.replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::null );

	data.replace( QString::fromLatin1( "&gt;" ), QString::fromLatin1( ">" ) );
	data.replace( QString::fromLatin1( "&lt;" ), QString::fromLatin1( "<" ) );
	data.replace( QString::fromLatin1( "&quot;" ), QString::fromLatin1( "\"" ) );
	data.replace( QString::fromLatin1( "&nbsp;" ), QString::fromLatin1( " " ) );
	data.replace( QString::fromLatin1( "&amp;" ), QString::fromLatin1( "&" ) );

	return data;
}

QString Message::escape( const QString &text )
{
	QString html = QStyleSheet::escape( text );
 	//Replace carriage returns inside the text
	html.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "<br />" ) );
	//Replace a tab with 4 spaces
	html.replace( QString::fromLatin1( "\t" ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

	//Replace multiple spaces with &nbsp;
	//do not replace every space so we break the linebreak
	html.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( "&nbsp; " ) );

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

	if( d->format & PlainText )
	{
		escapedBody=escape( escapedBody );
	}

	return escapedBody;
}

QString Message::parsedBody() const
{
	//kdDebug(14000) << k_funcinfo << "messageformat: " << d->format << endl;

	if( d->format == ParsedHTML )
	{
		return d->body;
	}
	else
	{
		return Kopete::Emoticons::parseEmoticons(parseLinks(escapedBody(), d->format));
	}
}

static QString makeRegExp( const char *pattern )
{
	const QString urlChar = QString::fromLatin1( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString boundaryStart = QString::fromLatin1( "(^|[^%1])(" ).arg( urlChar );
	const QString boundaryEnd = QString::fromLatin1( ")([^%1]|$)" ).arg( urlChar );

	return boundaryStart + QString::fromLatin1(pattern) + boundaryEnd;
}

QString Message::parseLinks( const QString &message, MessageFormat format )
{
	if ( format == ParsedHTML )
		return message;

	if ( format & RichText )
	{
		// < in HTML *always* means start-of-tag
		QStringList entries = QStringList::split( QChar('<'), message, true );

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
			int tagclose = curr.find( QChar('>') );
			// no >: the HTML is broken, but we can cope
			if ( tagclose == -1 )
				continue;
			QString tag = curr.left( tagclose + 1 );
			QString body = curr.mid( tagclose + 1 );
			*it = tag + parseLinks( body, PlainText );
		}
		return entries.join(QString::fromLatin1("<"));
	}

	QString result = message;

	// common subpatterns - may not contain matching parens!
	const QString name = QString::fromLatin1( "[\\w\\+\\-=_\\.]+" );
	const QString userAndPassword = QString::fromLatin1( "(?:%1(?::%1)?\\@)" ).arg( name );
	const QString urlChar = QString::fromLatin1( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString urlSection = QString::fromLatin1( "[%1]+" ).arg( urlChar );
	const QString domain = QString::fromLatin1( "[\\-\\w_]+(?:\\.[\\-\\w_]+)+" );

	//Replace http/https/ftp links:
	// Replace (stuff)://[user:password@](linkstuff) with a link
	result.replace(
		QRegExp( makeRegExp("\\w+://%1?\\w%2").arg( userAndPassword, urlSection ) ),
		QString::fromLatin1("\\1<a href=\"\\2\" title=\"\\2\">\\2</a>\\3" ) );

	// Replace www.X.Y(linkstuff) with a http: link
	result.replace(
		QRegExp( makeRegExp("%1?www\\.%2%3").arg( userAndPassword, domain, urlSection ) ),
		QString::fromLatin1("\\1<a href=\"http://\\2\" title=\"http://\\2\">\\2</a>\\3" ) );

	//Replace Email Links
	// Replace user@domain with a mailto: link
	result.replace(
		QRegExp( makeRegExp("%1@%2").arg( name, domain ) ),
		QString::fromLatin1("\\1<a href=\"mailto:\\2\" title=\"mailto:\\2\">\\2</a>\\3") );

	//Workaround for Bug 85061: Highlighted URLs adds a ' ' after the URL itself
	// the trailing  &nbsp; is included in the url.
	result.replace( QRegExp( QString::fromLatin1("(<a href=\"[^\"]+)(&nbsp;)(\")")  ) , QString::fromLatin1("\\1\\3") );

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

QPtrList<Contact> Message::to() const
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
	//QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	//return QFont( bodyNode.attribute( QString::fromLatin1("font") ), bodyNode.attribute( QString::fromLatin1("fontsize") ).toInt() );
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
	detach();
	d->manager=kmm;
}


QDomElement Message::contactNode( QDomDocument doc, const Contact *contact )
{
	KopetePrefs *p = KopetePrefs::prefs();

	// These colors are used for coloring nicknames. I tried to use
	// colors both visible on light and dark background.
	static const char* const nameColors[] =
	{
		"red", "blue" , "gray", "magenta", "violet", "olive", "yellowgreen",
		"darkred", "darkgreen", "darksalmon", "darkcyan", "darkyellow",
		"mediumpurple", "peru", "olivedrab", "royalred", "darkorange", "slateblue",
		"slategray", "goldenrod", "orangered", "tomato", "dogderblue", "steelblue",
		"deeppink", "saddlebrown", "coral", "royalblue"
	};

	static const int nameColorsLen = sizeof(nameColors) / sizeof(nameColors[0]) - 1;

	QString contactName = contact->property(Global::Properties::self()->nickName()).value().toString();
	if( p->truncateContactNames() )
	{
		contactName = KStringHandler::csqueeze( contactName, p->maxConactNameLength() );
	}

	if(contactName.isEmpty())
		contactName = contact->metaContact() ? contact->metaContact()->displayName() : contact->contactId();

	QString metacontactName = contact->metaContact() ? contact->metaContact()->displayName() : contactName;
	QDomElement contactNode = doc.createElement( QString::fromLatin1("contact") );
	contactNode.setAttribute( QString::fromLatin1("contactId"), contact->contactId() );

	QDomElement contactNameNode = doc.createElement( QString::fromLatin1("contactDisplayName") );
	contactNameNode.setAttribute( QString::fromLatin1("dir"), contactName.isRightToLeft() ?
	                              QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
	contactNameNode.setAttribute( QString::fromLatin1("text"), QStyleSheet::escape( contactName ) );
	contactNode.appendChild( contactNameNode );


	QDomElement metacontactNameNode = doc.createElement( QString::fromLatin1("metaContactDisplayName") );
	metacontactNameNode.setAttribute( QString::fromLatin1("dir"), metacontactName.isRightToLeft() ?
	                                  QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
	metacontactNameNode.setAttribute( QString::fromLatin1("text"), QStyleSheet::escape( metacontactName ) );

	if( contact->metaContact() )
	{
		QImage photo = contact->metaContact()->photo();
		if( !photo.isNull() )
		{
			QByteArray ba;
			QBuffer buffer( ba );
			buffer.open( IO_WriteOnly );
			photo.save ( &buffer, "PNG" );
			contactNode.setAttribute( QString::fromLatin1("userPhoto"), KCodecs::base64Encode(ba) );
		}
	}

	contactNode.appendChild( metacontactNameNode );

	// protocol() returns NULL here in the style preview in appearance config.
	// this isn't the right place to work around it, since contacts should never have
	// no protocol, but it works for now.
	QString iconName = QString::fromLatin1("kopete");
	if ( Protocol *protocol = contact->protocol() )
		iconName = protocol->pluginIcon();

	QString iconPath = KGlobal::iconLoader()->iconPath( iconName, KIcon::Small );
	contactNode.setAttribute( QString::fromLatin1("protocolIcon"), iconPath );

	// hash contactId to deterministically pick a color for the contact
	int hash = 0;
	const QString &contactId = contact->contactId();
	for( uint f = 0; f < contactId.length(); ++f )
		hash += contactId[f].latin1() * f;

	QString color = QColor( nameColors[ hash % nameColorsLen ] ).name();
	contactNode.setAttribute( QString::fromLatin1("color"), color );

	return contactNode;
}


const QDomDocument Message::asXML()
{
	QDomDocument doc;
	QDomElement messageNode = doc.createElement( QString::fromLatin1("message") );
	messageNode.setAttribute( QString::fromLatin1("time"),
		KGlobal::locale()->formatTime(d->timeStamp.time(), true) );
	messageNode.setAttribute( QString::fromLatin1("timestamp"),
		KGlobal::locale()->formatDateTime(d->timeStamp) );
	if( d->timeStamp.date() == QDate::currentDate() )
	{
		messageNode.setAttribute( QString::fromLatin1("formattedTimestamp"),
			KGlobal::locale()->formatTime(d->timeStamp.time(), true) );
	}
	else
	{
		messageNode.setAttribute( QString::fromLatin1("formattedTimestamp"),
			KGlobal::locale()->formatDateTime(d->timeStamp) );
	}
	messageNode.setAttribute( QString::fromLatin1("subject"), QStyleSheet::escape( d->subject ) );

	/**
	 * @deprecated backwards-compatibility direction attribute for old XSLT
	 * It used to be the case that Action was in the MessageDirection enum.
	 * Clearly this is broken - Action is not a direction, and it was impossible
	 * to tell whether actions were incoming or outgoing. Anyway, some XSLT view
	 * styles were written back when this was the case. They expected numeric
	 * 'directions'. So we fake some for them.
	 */
	{
		int oldDirection = 0;
		if( type() == TypeAction )
			oldDirection = 3;
		else if( direction() == Inbound )
			oldDirection = 0;
		else if( direction() == Outbound )
			oldDirection = 1;
		else if( direction() == Internal )
			oldDirection = 2;
		messageNode.setAttribute( QString::fromLatin1("direction"), oldDirection );
	}

	const char *route;
	switch( d->direction )
	{
		case Inbound:
			route = "inbound";
			break;
		case Outbound:
			route = "outbound";
			break;
		case Internal:
			route = "internal";
			break;
		default:
			kdWarning(14000) << k_funcinfo << "unknown message direction " << d->direction << endl;
			route = "unknown";
			break;
	}
	messageNode.setAttribute( QString::fromLatin1("route"), QString::fromLatin1(route) );

	const char *type;
	switch( d->type )
	{
		case TypeNormal:
			type = "normal";
			break;
		case TypeAction:
			type = "action";
			break;
		default:
			kdWarning(14000) << k_funcinfo << "unknown message type " << d->type << endl;
			type = "unknown";
			break;
	}
	messageNode.setAttribute( QString::fromLatin1("type"), QString::fromLatin1(type) );

	messageNode.setAttribute( QString::fromLatin1("importance"), d->importance );


	//build the <from> and <to>  node
	if( const Contact *mainContact = (d->direction == Inbound ? d->from : d->to.getFirst()) )
		messageNode.setAttribute( QString::fromLatin1("mainContactId"), mainContact->contactId() );

	doc.appendChild( messageNode );

	if( const Contact *c = d->from )
	{
		QDomElement fromNode = doc.createElement( QString::fromLatin1("from") );
		fromNode.appendChild( contactNode( doc, c ) );
		messageNode.appendChild( fromNode );
	}

	if( const Contact *c = d->to.getFirst() )
	{
		QDomElement toNode = doc.createElement( QString::fromLatin1("to") );
		toNode.appendChild( contactNode( doc, c ) );
		messageNode.appendChild( toNode );
	}

	QDomElement bodyNode = doc.createElement( QString::fromLatin1("body") );

	if( !d->fgOverride && d->fgColor.isValid() )
		bodyNode.setAttribute( QString::fromLatin1("color"), d->fgColor.name() );
	if( !d->bgOverride && d->bgColor.isValid() )
		bodyNode.setAttribute( QString::fromLatin1("bgcolor"), d->bgColor.name() );

	if( !d->rtfOverride && d->font!=QFont() )
	{
		QString fontstr;
		if(!d->font.family().isNull())
			fontstr+=QString::fromLatin1("font-family: ")+d->font.family()+QString::fromLatin1("; ");
		if(d->font.italic())
			fontstr+=QString::fromLatin1("font-style: italic; ");
		if(d->font.strikeOut())
			fontstr+=QString::fromLatin1("text-decoration: line-through; ");
		if(d->font.underline())
			fontstr+=QString::fromLatin1("text-decoration: underline; ");
		if(d->font.bold())
			fontstr+=QString::fromLatin1("font-weight: bold;");

		bodyNode.setAttribute( QString::fromLatin1("font"), fontstr );
	}

	bodyNode.setAttribute( QString::fromLatin1("dir"),
		plainBody().isRightToLeft() ? QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
	QDomCDATASection bodyText = doc.createCDATASection( parsedBody() );
	bodyNode.appendChild( bodyText );

	messageNode.appendChild( bodyNode );

	return doc;
}

QString Message::decodeString( const QCString &message, const QTextCodec *providedCodec, bool *success )
{
	/*
	Note to everyone. This function is not the most efficient, that is for sure.
	However, it *is* the only way we can be guarenteed that a given string is
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

	//They are providing a possible codec. Check if it is valid
	if( providedCodec && providedCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
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

	//Try codecForContent - exact match
	QTextCodec *testCodec = QTextCodec::codecForContent(message, charsToCheck);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		return testCodec->toUnicode( message );
	}

	kdWarning(14000) << k_funcinfo << "Unable to decode string using provided codec(s), taking best guesses!" << endl;
	if( success )
		*success = false;

	//We don't have any clues here.

	//Try local codec
	testCodec = QTextCodec::codecForLocale();
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kdDebug(14000) << k_funcinfo << "Using locale's codec" << endl;
		return testCodec->toUnicode( message );
	}

	//Try latin1 codec
	testCodec = QTextCodec::codecForMib(4);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kdDebug(14000) << k_funcinfo << "Using latin1" << endl;
		return testCodec->toUnicode( message );
	}

	kdDebug(14000) << k_funcinfo << "Using latin1 and cleaning string" << endl;
	//No codec decoded. Just decode latin1, and clean out any junk.
	QString result = testCodec->toUnicode( message );
	const uint length = message.length();
	for( uint i = 0; i < length; ++i )
	{
		if( !result[i].isPrint() )
			result[i] = '?';
	}

	return result;
}

