/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "kopetecontact.h"
#include "kopetemessage_imp.h"

Message::Message( KopeteMessage *m, QObject *parent,  const char *name )
	: BindingObject( parent, name ), msg( m ) {}

void Message::setBgColor( const QColor &color )
{
	kdDebug() << k_funcinfo << endl;
	msg->setBg( color );
}

QColor Message::bgColor() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->bg();
}

void Message::setFgColor( const QColor &color )
{
	kdDebug() << k_funcinfo << endl;
	msg->setFg( color );
}

QColor Message::fgColor() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->fg();
}

void Message::setFont( const QFont &font )
{
	kdDebug() << k_funcinfo << endl;
	msg->setFont( font );
}

QFont Message::font() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->font();
}

void Message::setPlainBody( const QString &body )
{
	kdDebug() << k_funcinfo << endl;
	msg->setBody( body, KopeteMessage::PlainText );
}

QString Message::plainBody() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->plainBody();
}

void Message::setRichBody( const QString &body )
{
	kdDebug() << k_funcinfo << endl;
	msg->setBody( body, KopeteMessage::RichText );
}

QString Message::richBody() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->parsedBody();
}

void Message::setImportance( int importance )
{
	kdDebug() << k_funcinfo << endl;
	msg->setImportance( (KopeteMessage::MessageImportance)importance );
}

int Message::importance() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->importance();
}

int Message::type() const
{
	kdDebug() << k_funcinfo << endl;
	return (int)msg->type();
}

QString Message::xml() const
{
	kdDebug() << k_funcinfo << endl;
	return msg->asXML().toString();
}

#include "kopetemessage_imp.moc"



