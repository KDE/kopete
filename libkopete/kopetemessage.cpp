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

#include <stdlib.h>

#include <qdatetime.h>
#include <qfont.h>
#include <qstylesheet.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmdcodec.h>

#include "kopeteemoticons.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopeteprefs.h"
#include "kopetexsl.h"

struct KopeteMessagePrivate
{
	uint refCount;

	const KopeteContact *from;
	KopeteContactPtrList to;
	QDomDocument xmlDoc;

	KopeteMessage::MessageDirection direction;
	KopeteMessage::MessageFormat format;
	KopeteMessage::MessageType type;
	KopeteMessage::MessageImportance importance;
	QDateTime timeStamp;

	bool bgOverride;
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
	if( other.d == d )
		return *this;

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
	if( color.isValid() )
	{
		detach();
		QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
		bodyNode.setAttribute( QString::fromLatin1("color"), color.name() );
	}
}

void KopeteMessage::setBg( const QColor &color )
{
	if( !d->bgOverride && color.isValid() )
	{
		detach();
		QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
		bodyNode.setAttribute( QString::fromLatin1("bgcolor"), color.name() );
	}
}

void KopeteMessage::setFont( const QFont &font )
{
	detach();
	QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	bodyNode.setAttribute( QString::fromLatin1("font"), font.family() );
	bodyNode.setAttribute( QString::fromLatin1("fontsize"), font.pointSize() );
}

void KopeteMessage::setBody( const QString &body, MessageFormat f )
{
	detach();
	QDomCDATASection bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).firstChild().toCDATASection();
	bodyNode.setData( body );
	d->format = f;
}

void KopeteMessage::setImportance(KopeteMessage::MessageImportance i)
{
	detach();
	d->importance = i;
	d->xmlDoc.documentElement().setAttribute( QString::fromLatin1("importance"), i );
}

void KopeteMessage::init( const QDateTime &timeStamp, const KopeteContact *from, const KopeteContactPtrList &to,
	const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, MessageType type )
{
	static QMap<QString,QColor> colorMap;
	static int lastColor;
	const QColor nameColors[] = {
		Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta,
		Qt::darkRed, Qt::darkGreen, Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow
	};

	d->refCount = 1;
	d->from = from;
	d->to   = to;
	d->importance = (to.count() <= 1) ? Normal : Low;
	d->timeStamp = timeStamp;

	QDomElement messageNode = d->xmlDoc.createElement( QString::fromLatin1("message") );
	messageNode.setAttribute( QString::fromLatin1("time"), KGlobal::locale()->formatTime(timeStamp.time(), true) );
	messageNode.setAttribute( QString::fromLatin1("timestamp"), timeStamp.toString() );
	messageNode.setAttribute( QString::fromLatin1("subject"), subject );
	messageNode.setAttribute( QString::fromLatin1("direction"), direction );
	messageNode.setAttribute( QString::fromLatin1("importance"), d->importance );
	if( from )
		messageNode.setAttribute( QString::fromLatin1("mainContactId"), direction == Inbound ? d->from->contactId() : d->to.first()->contactId() );

	d->xmlDoc.appendChild( messageNode );

	if( from )
	{
		QDomElement fromNode = d->xmlDoc.createElement( QString::fromLatin1("from") );
		QDomElement fromContactNode = d->xmlDoc.createElement( QString::fromLatin1("contact") );
		fromContactNode.setAttribute( QString::fromLatin1("contactId"), from->contactId() );
		fromContactNode.setAttribute( QString::fromLatin1("contactDisplayName"), from->displayName() );
		QString fromName = from->metaContact() ? from->metaContact()->displayName() : from->displayName();
		fromContactNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), fromName );
		fromNode.appendChild( fromContactNode );

		if( !colorMap.contains( fromName ) )
		{
			QColor newColor;
			if( direction == Outbound )
				newColor = Qt::yellow;
			else
				newColor = nameColors[(lastColor++) % (sizeof(nameColors) / sizeof(nameColors[0]))];
			colorMap.insert( fromName, newColor );
		}
		fromContactNode.setAttribute( QString::fromLatin1("color"), colorMap[ fromName ].name() );
		messageNode.appendChild( fromNode );
	}

	QDomElement toNode = d->xmlDoc.createElement( QString::fromLatin1("to") );
	for( KopeteContact *c = d->to.first(); c; c = d->to.next() )
	{
		QDomElement cNode = d->xmlDoc.createElement( QString::fromLatin1("contact") );
		cNode.setAttribute( QString::fromLatin1("contactId"), c->contactId() );
		cNode.setAttribute( QString::fromLatin1("contactDisplayName"), c->displayName() );
		cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), c->metaContact() ? c->metaContact()->displayName() : c->displayName() );
		toNode.appendChild( cNode );
	}
	messageNode.appendChild( toNode );

	QDomElement bodyNode = d->xmlDoc.createElement( QString::fromLatin1("body") );
	QDomCDATASection bodyText = d->xmlDoc.createCDATASection( body );
	bodyNode.appendChild( bodyText );

	messageNode.appendChild( bodyNode );

	d->format = f;
	d->bgOverride = false;
	d->type = type;
}

QString KopeteMessage::plainBody() const
{
	QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();

	if( d->format == PlainText )
		return bodyText.text();
	else
		return KopeteXSL::unescape( bodyText.text() ).replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::fromLatin1( "" ) );
}

QString KopeteMessage::escapedBody() const
{
	QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	QString escapedBody = bodyText.text();

	if( d->format == PlainText )
	{
		escapedBody = QStyleSheet::escape( escapedBody );

		//Replace carriage returns inside the text
		escapedBody.replace( QRegExp( QString::fromLatin1( "\n" ) ), QString::fromLatin1( "<br/>" ) );

		//Replace a tab with 4 spaces
		escapedBody.replace( QRegExp( QString::fromLatin1( "\t" ) ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

		//Replace multiple spaces with &nbsp;
		escapedBody.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( "&nbsp;&nbsp;" ) );
	}

	return escapedBody;
}

QString KopeteMessage::parsedBody() const
{
	QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	QString parsedBody = bodyText.text();

	if( d->format == ParsedHTML )
		return parsedBody;
	else
		return KopeteEmoticons::parseEmoticons(parseLinks(escapedBody()));
}

QString KopeteMessage::parseLinks( const QString &message ) const
{
	QString result = message;

	//Replace Email Links
	result.replace( QRegExp( QString::fromLatin1("\\b([\\w-_\\.]+@([-_\\w\\.]+\\.\\w+)+)\\b") ), QString::fromLatin1("<a href=\"mailto:\\1\">\\1</a>") );

	//Replace http/https/ftp links
	result.replace( QRegExp( QString::fromLatin1("\\b((http://\\w|ftp://\\w|https://\\w|www\\.)[\\w\\.-_]+[\\w\\./#&;:=\\?~%-_,]*)\\b") ), QString::fromLatin1("<a href=\"\\1\">\\1</a>" ) );

	return result;
}

const QDomDocument KopeteMessage::asXML() const
{
	QDomDocument doc = d->xmlDoc.cloneNode().toDocument();
	QDomCDATASection bodyText = doc.elementsByTagName( QString::fromLatin1("body") ).item(0).firstChild().toCDATASection();
	bodyText.setData( parsedBody() );
	return doc;
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
	return QColor( d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement().attribute( QString::fromLatin1("color") ) );
}

QColor KopeteMessage::bg() const
{
	return QColor( d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement().attribute( QString::fromLatin1("bgcolor") ) );
}

QFont KopeteMessage::font() const
{
	QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	return QFont( bodyNode.attribute( QString::fromLatin1("font") ), bodyNode.attribute( QString::fromLatin1("font") ).toInt() );
}

QString KopeteMessage::subject() const
{
	return d->xmlDoc.documentElement().attribute( QString::fromLatin1("subject") );
}

KopeteMessage::MessageFormat KopeteMessage::format() const
{
	return d->format;
}

KopeteMessage::MessageDirection KopeteMessage::direction() const
{
	return d->direction;
}

KopeteMessage::MessageImportance KopeteMessage::importance() const
{
	return d->importance;
}

void KopeteMessage::detach()
{
	if( d->refCount == 1 )
		return;

	// Warning: this only works as long as the private object doesn't contain pointers to allocated objects.
	// The from contact for example is fine, but it's a shallow copy this way.
	KopeteMessagePrivate *newD = new KopeteMessagePrivate(*d);
	newD->refCount = 1;
	d->refCount--;

	d = newD;
}

// vim: set noet ts=4 sts=4 sw=4:

