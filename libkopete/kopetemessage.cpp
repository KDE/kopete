/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qstylesheet.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kopeteemoticons.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"


//
// This define say if the message is stored internaly as a QDomDocument, or as simples element.
// Adventage:
//   - this is a good desing,  the objective is that all kopete use XML
// Inconvenient
//   - It is slower to make operation which are not XML based  (plugins)
//   - It take (few) more memory when we have to keep the message in memory
//
// Currentlu, the QDomDocument id *NEVER* used as QDomDocument,  even in the chatwindow which is the
// ONLY place where the XML is used, XSLT requiere string based XML,  so currently, i think it is
// preferable to don't use QDom for storing
//
#ifndef MESSAGE_QDOM
#define MESSAGE_QDOM 0
#endif

struct KopeteMessagePrivate
{
	uint refCount;

	const KopeteContact *from;
	KopeteMessageManager *manager;
	KopeteContactPtrList to;

	KopeteMessage::MessageDirection direction;
	KopeteMessage::MessageFormat format;
	KopeteMessage::MessageType type;
	KopeteMessage::MessageImportance importance;
	bool bgOverride;

	QDateTime timeStamp;
	QFont font;

#if MESSAGE_QDOM
	QDomDocument xmlDoc;
#else
	QColor fgColor;
	QColor bgColor;
	QString body;
	QString subject;
#endif

	static QMap<QString,QColor> colorMap;
	static int lastColor;
};

QMap<QString,QColor> KopeteMessagePrivate::colorMap = QMap<QString,QColor>();
int KopeteMessagePrivate::lastColor=0;


static const QColor nameColors[] = {
	Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta,
	Qt::darkRed, Qt::darkGreen, Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow
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

KopeteMessage::KopeteMessage( const KopeteContact *fromKC, const KopeteContact *toKC, const QString &body,
	MessageDirection direction, MessageFormat f, MessageType type )
{
	d = new KopeteMessagePrivate;
	KopeteContactPtrList to;
	to.append(toKC);
	init( QDateTime::currentDateTime(), fromKC, to, body, QString::null, direction, f, type );
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


void KopeteMessage::init( const QDateTime &timeStamp, const KopeteContact *from, const KopeteContactPtrList &to,
	const QString &body, const QString &subject, MessageDirection direction, MessageFormat f, MessageType type )
{
	d->refCount = 1;
	d->from = from;
	d->to   = to;
	d->importance = (to.count() <= 1) ? Normal : Low;
	d->timeStamp = timeStamp;
	d->direction = direction;
	d->manager=0l;
	d->format = f;
	d->bgOverride = false;
	d->type = type;


#if MESSAGE_QDOM
	QDomElement messageNode = d->xmlDoc.createElement( QString::fromLatin1("message") );
	messageNode.setAttribute( QString::fromLatin1("time"), KGlobal::locale()->formatTime(timeStamp.time(), true) );
	messageNode.setAttribute( QString::fromLatin1("timestamp"), timeStamp.toString() );
	messageNode.setAttribute( QString::fromLatin1("subject"), QStyleSheet::escape( subject ) );
	messageNode.setAttribute( QString::fromLatin1("direction"), direction );
	messageNode.setAttribute( QString::fromLatin1("importance"), d->importance );
#else
	d->subject=subject;
#endif

#if MESSAGE_QDOM
	//build the <from> and <to>  node
	if( from )
		messageNode.setAttribute( QString::fromLatin1("mainContactId"), direction == Inbound ? d->from->contactId() : d->to.first()->contactId() );

	d->xmlDoc.appendChild( messageNode );

	if( from )
	{

		QDomElement fromNode = d->xmlDoc.createElement( QString::fromLatin1("from") );
		QDomElement fromContactNode = d->xmlDoc.createElement( QString::fromLatin1("contact") );
		fromContactNode.setAttribute( QString::fromLatin1("contactId"), from->contactId() );
		fromContactNode.setAttribute( QString::fromLatin1("contactDisplayName"), QStyleSheet::escape( from->displayName() ) );
		QString fromName = from->metaContact() ? from->metaContact()->displayName() : from->displayName();
		fromContactNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), QStyleSheet::escape( fromName ) );
		fromNode.appendChild( fromContactNode );

		if( !d->colorMap.contains( fromName ) )
		{
			QColor newColor;
			if( direction == Outbound )
				newColor = Qt::yellow;
			else
				newColor = nameColors[(d->lastColor++) % (sizeof(nameColors) / sizeof(nameColors[0]))];
			d->colorMap.insert( fromName, newColor );
		}
		fromContactNode.setAttribute( QString::fromLatin1("color"), d->colorMap[ fromName ].name() );
		messageNode.appendChild( fromNode );
	}

	QDomElement toNode = d->xmlDoc.createElement( QString::fromLatin1("to") );
	///for( KopeteContact *c = d->to.first(); c; c = d->to.next() )
	//{
		KopeteContact *c = d->to.first();
		if( c )
		{
			QDomElement cNode = d->xmlDoc.createElement( QString::fromLatin1("contact") );
			cNode.setAttribute( QString::fromLatin1("contactId"), c->contactId() );
			cNode.setAttribute( QString::fromLatin1("contactDisplayName"), QStyleSheet::escape( c->displayName() ) );
			cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), c->metaContact() ?
				QStyleSheet::escape( c->metaContact()->displayName() ) : QStyleSheet::escape( c->displayName() ) );
			toNode.appendChild( cNode );
		}
	//}
	messageNode.appendChild( toNode );

#endif

	//Make the body correct

	QString theBody = body;
	if( f == RichText )
	{
		//This is coming from the RichTextEditor component.
		//Strip off the containing HTML document
		theBody.replace( QRegExp( QString::fromLatin1(".*<body.*>\\s+(.*)\\s+</body>.*") ), QString::fromLatin1("\\1") );

		//Strip <p> tags
		theBody.replace( QString::fromLatin1("<p>"), QString::null );

		//Replace </p> with a <br/>
		theBody.replace( QString::fromLatin1("</p>") , QString::fromLatin1("<br/>") );

		//Remove trailing <br/>
		if ( theBody.endsWith( QString::fromLatin1("<br/>") ) )
			theBody.truncate( theBody.length() - 5 );
		theBody.remove(  QString::fromLatin1("\n") );
	}

#if MESSAGE_QDOM
	QDomElement bodyNode = d->xmlDoc.createElement( QString::fromLatin1("body") );
	QDomCDATASection bodyText = d->xmlDoc.createCDATASection( theBody );
	bodyNode.appendChild( bodyText );

	messageNode.appendChild( bodyNode );
#else
	d->body=theBody;
#endif
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



void KopeteMessage::setBgOverride( bool enabled )
{
	detach();
	d->bgOverride = enabled;
}

void KopeteMessage::setFg( const QColor &color )
{
	detach();
#if MESSAGE_QDOM
	QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	if( color.isValid() )
	{
		bodyNode.setAttribute( QString::fromLatin1("color"), color.name() );
	}
	else
	{
		bodyNode.removeAttribute( QString::fromLatin1("color") );
	}
#else
	d->fgColor=color;
#endif
}

void KopeteMessage::setBg( const QColor &color )
{
	detach();
#if MESSAGE_QDOM
	QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	if( !d->bgOverride && color.isValid() )
		bodyNode.setAttribute( QString::fromLatin1("bgcolor"), color.name() );
	else
		bodyNode.removeAttribute( QString::fromLatin1("bgcolor") );
#else
	d->bgColor=color;
#endif
}

void KopeteMessage::setFont( const QFont &font )
{
	detach();
	d->font = font;

#if MESSAGE_QDOM
	QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();

	if(font!=QFont())
	{
		QString fontstr;
		if(!font.family().isNull())
			fontstr+=QString::fromLatin1("font-family: ")+font.family()+QString::fromLatin1("; ");
		if(font.italic())
			fontstr+=QString::fromLatin1("font-style: italic; ");
		if(font.strikeOut())
			fontstr+=QString::fromLatin1("text-decoration: line-through; ");
		if(font.underline())
			fontstr+=QString::fromLatin1("text-decoration: underline; ");
		if(font.bold())
			fontstr+=QString::fromLatin1("font-weight: bold;");

		//TODO: font size
//		kdDebug() << k_funcinfo << fontstr <<endl;
		bodyNode.setAttribute( QString::fromLatin1("font"), fontstr );
		//bodyNode.setAttribute( QString::fromLatin1("fontsize"), font.pointSize() );
	}
	else
		bodyNode.removeAttribute( QString::fromLatin1("font") );
#endif
}

void KopeteMessage::setBody( const QString &body, MessageFormat f )
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

#if MESSAGE_QDOM
	QDomCDATASection bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).firstChild().toCDATASection();
	bodyNode.setData( theBody );
#else
	d->body=body;
#endif

	d->format = f;
}

void KopeteMessage::setImportance(KopeteMessage::MessageImportance i)
{
	detach();
	d->importance = i;
#if MESSAGE_QDOM
	d->xmlDoc.documentElement().setAttribute( QString::fromLatin1("importance"), i );
#endif
}

QString KopeteMessage::unescape( const QString &xml )
{
	QString data = xml;

	//Can someone (Jason?) explain to me why unescaping "" to " ?? thanks - Olivier
	data.replace( QString::fromLatin1( "\"\"" ), QString::fromLatin1( "\"" ) );
	data.replace( QString::fromLatin1( "&gt;" ), QString::fromLatin1( ">" ) );
	data.replace( QString::fromLatin1( "&lt;" ), QString::fromLatin1( "<" ) );
	data.replace( QString::fromLatin1( "&quot;" ), QString::fromLatin1( "\"" ) );
	data.replace( QString::fromLatin1( "&nbsp;" ), QString::fromLatin1( " " ) );
	data.replace( QString::fromLatin1( "&amp;" ), QString::fromLatin1( "&" ) );

	return data;
}

QString KopeteMessage::plainBody() const
{
#if MESSAGE_QDOM
	QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	QString body=bodyText.text();
#else
	QString body=d->body;
#endif

	if( d->format & RichText )
	{
		body.replace( QRegExp( QString::fromLatin1( "< *br */? *>" ) , false ), QString::fromLatin1( "\n" ) );
		body.replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::null );
		body = unescape( body );
	}
	return body;
}

QString KopeteMessage::escapedBody() const
{
#if MESSAGE_QDOM
	QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	QString escapedBody = bodyText.text();
#else
	QString escapedBody=d->body;
#endif

	if( d->format & PlainText )
	{
		escapedBody = QStyleSheet::escape( escapedBody );
 		//Replace carriage returns inside the text
		escapedBody.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "<br/>" ) );

		//Replace a tab with 4 spaces
		escapedBody.replace( QString::fromLatin1( "\t" ), QString::fromLatin1( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

		//Replace multiple spaces with &nbsp;  //do not remplace everyspace so we break the linebreak
		escapedBody.replace( QRegExp( QString::fromLatin1( "\\s\\s" ) ), QString::fromLatin1( "&nbsp; " ) );
	}

	return escapedBody;
}

QString KopeteMessage::parsedBody() const
{
	if( d->format == ParsedHTML )
	{
#if MESSAGE_QDOM
		QDomElement bodyText = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
		return bodyText.text();
#else
		return d->body;
#endif
	}
	else
	{
		return KopeteEmoticons::parseEmoticons(parseLinks(escapedBody()));
	}
}

QString KopeteMessage::parseLinks( const QString &message ) const
{
	QString result = message;

	//Replace http/https/ftp links
	result.replace( QRegExp( QString::fromLatin1("(?:\\b|&nbsp;)((?:http(?:s?)://\\w|ftp://\\w)[-\\w\\._]+[-\\w\\./#&;:=\\?~%_,]*)(?:\\b|&nbsp;)") ), QString::fromLatin1("<a href=\"\\1\" title=\"\\1\">\\1</a>" ) );
	result.replace( QRegExp( QString::fromLatin1("^(www\\.[-\\w\\._]+[-\\w\\./#&;:=\\?~%_,]*)(?:\\b|&nbsp;)") ), QString::fromLatin1("<a href=\"http://\\1\" title=\"http://\\1\">\\1</a>" ) );
	result.replace( QRegExp( QString::fromLatin1("([^/.;,?#=\\-%~])\\b(www\\.[-\\w\\._]+[-\\w\\./#&;:=\\?~%_,]*)(?:\\b|&nbsp;)") ), QString::fromLatin1("\\1<a href=\"http://\\2\" title=\"http://\\2\">\\2</a>" ) );

	//Replace Email Links
	result.replace( QRegExp( QString::fromLatin1("(?:\\b|&nbsp;)([\\w\\-_\\.]+@(?:[-_\\w\\.]+\\.\\w+)+)(?:\\b|&nbsp;)") ), QString::fromLatin1("<a href=\"mailto:\\1\" title=\"mailto:\\1\">\\1</a>") );

	return result;
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
#if MESSAGE_QDOM
	return QColor( d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement().attribute( QString::fromLatin1("color") ) );
#else
	return d->fgColor;
#endif
}

QColor KopeteMessage::bg() const
{
#if MESSAGE_QDOM
	return QColor( d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement().attribute( QString::fromLatin1("bgcolor") ) );
#else
	return d->bgColor;
#endif
}

QFont KopeteMessage::font() const
{
	//QDomElement bodyNode = d->xmlDoc.elementsByTagName( QString::fromLatin1("body") ).item(0).toElement();
	//return QFont( bodyNode.attribute( QString::fromLatin1("font") ), bodyNode.attribute( QString::fromLatin1("fontsize") ).toInt() );
	return d->font;
}

QString KopeteMessage::subject() const
{
#if MESSAGE_QDOM
	return d->xmlDoc.documentElement().attribute( QString::fromLatin1("subject") );
#else
	return d->subject;
#endif
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

KopeteMessageManager *KopeteMessage::manager() const
{
	return d->manager;
}

void KopeteMessage::setManager(KopeteMessageManager *kmm)
{
	detach();
	d->manager=kmm;
}



const QDomDocument KopeteMessage::asXML() const
{
#if MESSAGE_QDOM
	QDomDocument doc = d->xmlDoc.cloneNode().toDocument();
	QDomCDATASection bodyText = doc.elementsByTagName( QString::fromLatin1("body") ).item(0).firstChild().toCDATASection();
	bodyText.setData( parsedBody() );
	return doc;
#else

	QDomDocument doc;
	QDomElement messageNode = doc.createElement( QString::fromLatin1("message") );
	messageNode.setAttribute( QString::fromLatin1("time"), KGlobal::locale()->formatTime(d->timeStamp.time(), true) );
	messageNode.setAttribute( QString::fromLatin1("timestamp"), d->timeStamp.toString() );
	messageNode.setAttribute( QString::fromLatin1("subject"), QStyleSheet::escape( d->subject ) );
	messageNode.setAttribute( QString::fromLatin1("direction"), d->direction );
	messageNode.setAttribute( QString::fromLatin1("importance"), d->importance );


	//build the <from> and <to>  node
	if( d->direction == Inbound ? d->from : d->to.first() )
		messageNode.setAttribute( QString::fromLatin1("mainContactId"), d->direction == Inbound ? d->from->contactId() : d->to.first()->contactId() );

	doc.appendChild( messageNode );

	if( d->from )
	{
		QDomElement fromNode = doc.createElement( QString::fromLatin1("from") );
		QDomElement fromContactNode = doc.createElement( QString::fromLatin1("contact") );
		fromContactNode.setAttribute( QString::fromLatin1("contactId"), d->from->contactId() );
		fromContactNode.setAttribute( QString::fromLatin1("contactDisplayName"), QStyleSheet::escape( d->from->displayName() ) );
		QString fromName = d->from->metaContact() ? d->from->metaContact()->displayName() : d->from->displayName();
		fromContactNode.setAttribute( QString::fromLatin1("metaContactDisplayName"),  fromName  );
		fromNode.appendChild( fromContactNode );

		if( !d->colorMap.contains( fromName ) )
		{
			QColor newColor;
			if( d->direction == Outbound )
				newColor = Qt::yellow;
			else
				newColor = nameColors[(d->lastColor++) % (sizeof(nameColors) / sizeof(nameColors[0]))];
			d->colorMap.insert( fromName, newColor );
		}
		fromContactNode.setAttribute( QString::fromLatin1("color"), d->colorMap[ fromName ].name() );
		messageNode.appendChild( fromNode );
	}

	QDomElement toNode = doc.createElement( QString::fromLatin1("to") );
	///for( KopeteContact *c = d->to.first(); c; c = d->to.next() )
	//{
		KopeteContact *c = d->to.first();
		if( c )
		{
			QDomElement cNode = doc.createElement( QString::fromLatin1("contact") );
			cNode.setAttribute( QString::fromLatin1("contactId"), c->contactId() );
			cNode.setAttribute( QString::fromLatin1("contactDisplayName"), QStyleSheet::escape( c->displayName() ) );
			cNode.setAttribute( QString::fromLatin1("metaContactDisplayName"), c->metaContact() ?
				QStyleSheet::escape( c->metaContact()->displayName() ) : QStyleSheet::escape( c->displayName() ) );
			toNode.appendChild( cNode );
		}
	//}
	messageNode.appendChild( toNode );

	QDomElement bodyNode = doc.createElement( QString::fromLatin1("body") );

	if( d->fgColor.isValid() )
		bodyNode.setAttribute( QString::fromLatin1("color"), d->fgColor.name() );
	if( !d->bgOverride && d->bgColor.isValid() )
		bodyNode.setAttribute( QString::fromLatin1("bgcolor"), d->bgColor.name() );

	if(d->font!=QFont())
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

	QDomCDATASection bodyText = doc.createCDATASection( parsedBody() );
	bodyNode.appendChild( bodyText );

	messageNode.appendChild( bodyNode );

	return doc;

#endif
}


// vim: set noet ts=4 sts=4 sw=4:

