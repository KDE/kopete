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

#include "kopeteemoticons.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteaccount.h"
#include "kopeteprefs.h"
#include "kopetexsl.h"

struct KopeteMessagePrivate
{
	uint refCount;

	const KopeteContact *from;
	KopeteContactPtrList to;
	QColor bgColor;
	QColor fgColor;
	QColor contactColor;
	QDomDocument xmlDoc;
	bool contentsModified;
	bool highlighted;
	QDateTime timeStamp;
	QFont font;
	QString body;
	QString subject;
	KopeteMessage::MessageDirection direction;
	KopeteMessage::MessageFormat format;
	KopeteMessage::MessageType type;
	KopeteMessage::MessageImportance importance;

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
	d->contentsModified = true;
}

void KopeteMessage::setFg( const QColor &color )
{
	detach();
	d->fgColor = color;
	compareColors( d->fgColor, d->bgColor );
	d->contentsModified = true;
}

void KopeteMessage::setBg( const QColor &color )
{
	detach();
	d->bgColor = color;
	compareColors( d->fgColor, d->bgColor );
	compareColors( d->contactColor, d->bgColor );
	d->contentsModified = true;
}

void KopeteMessage::compareColors( QColor &colorFg, QColor &colorBg )
{
	int h1, s1, v1, h2, s2, v2, vDiff;
	colorFg.hsv( &h1, &s1, &v1 );
	colorBg.hsv( &h2, &s2, &v2 );
	vDiff = v1 - v2;

	if( h1 == s1 && h2 == s2 && ( abs( vDiff ) <= 150 ) )
		colorFg = QColor( h2, s2, (v1 + 127) % 255, QColor::Hsv );
}

void KopeteMessage::setFont( const QFont &font )
{
	detach();
	d->font = font;
	d->contentsModified = true;
}

void KopeteMessage::highlight()
{
	detach();
	d->importance = Highlight;
	d->contentsModified = true;
}

void KopeteMessage::setBody( const QString &body, MessageFormat f )
{
	detach();
	d->body = body;
	d->format = f;
	d->contentsModified = true;
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
	//Importance to low in a multi chat
	d->importance= (to.count() <= 1) ? Normal : Low ;

	if( from )
	{
		QString fromName = d->from->metaContact() ? d->from->metaContact()->displayName() : d->from->displayName();

		if( !colorMap.contains( fromName ) )
		{
			QColor newColor;
			if( direction == Outbound )
				newColor = Qt::yellow;
			else
				newColor = nameColors[(lastColor++) % (sizeof(nameColors) / sizeof(nameColors[0]))];
			colorMap.insert( fromName, newColor );
		}
		d->contactColor = colorMap[ fromName ];

		//Highlight if the message contains the nickname (i think it should be place in the highlight plugin)
		if( KopetePrefs::prefs()->highlightEnabled() && from->account() && from->account()->myself() &&
			d->body.contains( QRegExp(QString::fromLatin1("\\b(%1)\\b").arg(from->account()->myself()->displayName()),false) ) )
		{
			highlight();
		}
	}

	d->contentsModified = true;
}

QString KopeteMessage::plainBody() const
{
	if( d->format & PlainText )
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
		QString parsedString = d->body;

		parsedString = QStyleSheet::escape( parsedString );

		//Replace carriage returns inside the text
		parsedString.replace( QRegExp( QString::fromLatin1( "\n" ) ), QString::fromLatin1( "<br/>" ) );

		//Replace a tab with 4 spaces
		parsedString.replace( QRegExp( QString::fromLatin1( "\t" ) ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

		//Replace multiple spaces with &nbsp;
		parsedString.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( "&nbsp;&nbsp;" ) );

		return parsedString;
	}

	return d->body;
}

QString KopeteMessage::parsedBody() const
{
	if( d->format == ParsedHTML )
		return d->body;

	return KopeteEmoticons::parseEmoticons(parseLinks(escapedBody()));
}

QString KopeteMessage::parseLinks( const QString &message ) const
{
	QString result = message;

	//Replace Email Links
	result.replace( QRegExp( QString::fromLatin1("\\b([\\w\\.]+@([\\w\\.]+\\.\\w+)+)\\b") ), QString::fromLatin1("<a href=\"mailto:\\1\">\\1</a>") );

	//Replace http/https/ftp links
	result.replace( QRegExp( QString::fromLatin1("\\b((http://\\w|ftp://\\w|https://\\w|www\\.)[\\w\\.]+[\\w\\./#&;:=\\?]*)\\b") ), QString::fromLatin1("<a href=\"\\1\">\\1</a>" ) );

	return result;
}

QDomDocument KopeteMessage::asXML()
{
	if( !d->xmlDoc.hasChildNodes() || d->contentsModified )
	{
		QDomDocument doc;
		QDomElement messageNode = doc.createElement( QString::fromLatin1("message") );
		messageNode.setAttribute( QString::fromLatin1("timestamp"), KGlobal::locale()->formatTime(d->timeStamp.time(), true) );
		messageNode.setAttribute( QString::fromLatin1("importance"), d->importance );
		messageNode.setAttribute( QString::fromLatin1("subject"), d->subject );
		messageNode.setAttribute( QString::fromLatin1("direction"), d->direction );
		doc.appendChild( messageNode );

		QDomElement fromNode = doc.createElement( QString::fromLatin1("from") );
		QDomElement cNode = doc.createElement( QString::fromLatin1("contact") );
		cNode.setAttribute( QString::fromLatin1("contactDisplayName"), d->from->displayName() );
		cNode.setAttribute( QString::fromLatin1("color"), d->contactColor.name() );
		if( d->from->metaContact() )
			cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), d->from->metaContact()->displayName() );
		else
			cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), d->from->displayName() );
		fromNode.appendChild( cNode );

		messageNode.setAttribute( QString::fromLatin1("from"), d->from->displayName() );

		QDomElement toNode = doc.createElement( QString::fromLatin1("to") );
		for( KopeteContact *c = d->to.first(); c; c = d->to.next() )
		{
			QDomElement cNode = doc.createElement( QString::fromLatin1("contact") );
			cNode.setAttribute( QString::fromLatin1("contactDisplayName"), c->displayName() );
			if( c->metaContact() )
				cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), c->metaContact()->displayName() );
			else
				cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), c->displayName() );
			toNode.appendChild( cNode );
		}

		messageNode.appendChild( fromNode );
		messageNode.appendChild( toNode );

		QDomElement bodyNode = doc.createElement( QString::fromLatin1("body") );
		if( !d->bgOverride && d->bgColor.isValid() )
			bodyNode.setAttribute( QString::fromLatin1("bgcolor"), d->bgColor.name() );
		if( d->fgColor.isValid() )
			bodyNode.setAttribute( QString::fromLatin1("color"), d->fgColor.name() );
		bodyNode.setAttribute( QString::fromLatin1("font"), d->font.family() );

		QDomCDATASection bodyText = doc.createCDATASection( KopeteEmoticons::parseEmoticons(parseLinks( escapedBody() )) );
		bodyNode.appendChild( bodyText );

		messageNode.appendChild( bodyNode );

		d->xmlDoc = doc;
		d->contentsModified = false;
	}

	return d->xmlDoc;
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

KopeteMessage::MessageImportance KopeteMessage::importance() const
{
	return d->importance;
}

void KopeteMessage::setImportance(KopeteMessage::MessageImportance i)
{
	d->importance=i;
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

