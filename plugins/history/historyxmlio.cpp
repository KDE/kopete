/*
    historyxmlio.cpp

    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historyxmlio.h"

#include "history.h"
#include <qdebug.h>
#include <QDateTime>
#include <QIODevice>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtCore/qstring.h>
#include <QStringRef>




static void writeHeader( const History &history, QXmlStreamWriter & writer )
{
//  qDebug() << "entered write header writing dates";
  writer.writeStartElement( QLatin1String( "head" ) );
  
    writer.writeStartElement( QLatin1String( "date" ) );
//    qDebug() << history.date().year()<<history.date().month();
//    qDebug() << QString::number(history.date().month() );
    QString month_s = QString::number(history.date().month() );
//    qDebug() << month_s;
    writer.writeAttribute( QLatin1String( "month" ), QString::number( history.date().month() ) );
    writer.writeAttribute( QLatin1String( "year" ),QString::number( history.date().year() ) );
    writer.writeEndElement();
    
    writer.writeStartElement( QLatin1String( "contact" ) );
    writer.writeAttribute( QLatin1String( "contactId" ), history.localContactId() );
    writer.writeAttribute( QLatin1String( "type" ), QLatin1String( "myself" ) );
    writer.writeEndElement();

    writer.writeStartElement( QLatin1String( "contact" ) );
    writer.writeAttribute( QLatin1String( "contactId" ), history.remoteContactId() );
    writer.writeEndElement();

  writer.writeEndElement();
}

static void writeMessages( const History &history, QXmlStreamWriter & writer )
{
//  qDebug() << "entered writeMessages";
  //  writer.writeStartElement( QLatin1String( "messages" ) );

  const History::Message::List messages = history.messages();
//  if ( history.messages().isEmpty() ) qDebug() <<" messages empty" ;
  foreach ( const History::Message &message, messages ) {
//    qDebug() << " entered foreach loop " ;
    writer.writeStartElement( QLatin1String( "msg" ) );
      writer.writeAttribute( QLatin1String( "nick" ), message.nick() );
      writer.writeAttribute( QLatin1String( "in" ), message.in() );
      writer.writeAttribute( QLatin1String( "from" ), message.sender() );
      writer.writeAttribute( QLatin1String( "time" ),
			     message.timestamp().toString( "d h:m:s" ) );
      writer.writeCharacters( message.text() );
    writer.writeEndElement();
  }
//  qDebug() << " exiting write message";
//  writer.writeEndElement();
}

bool HistoryXmlIo::writeHistoryToXml( const History &history, QIODevice *device )
{
  if ( device == 0 || !device->isWritable() )
    return false;

  QXmlStreamWriter writer( device );
  writer.setAutoFormatting( true );

  writer.writeStartDocument();

  writer.writeStartElement( QLatin1String( "kopete-history" ) );

  writeHeader( history, writer );
  writeMessages( history, writer );

  writer.writeEndElement();

  writer.writeEndDocument();

  return true;
}

static bool readHeader( QXmlStreamReader &reader, History &history )
{
  bool seenHeader = false;
  bool seenlocalcontact = false;
  while ( !reader.atEnd() ) {
    reader.readNext();

    if ( reader.isStartElement() ) {
      if ( reader.name() == QLatin1String( "head" ) ) {// qDebug() << " head detected";
        seenHeader = true;
      }	else if (reader.name() == QLatin1String( "date" ) ) {
//	qDebug() <<"date detected";
//	bool testmonth, testyear;

	const QXmlStreamAttributes attributes = reader.attributes();	
	const QStringRef s_month = attributes.value( QLatin1String( "month" ) );
	const QStringRef s_years = attributes.value( QLatin1String( "year" ) );
	
//	if ( s_years.isEmpty() ) qDebug() << " year is empty";
	
//	qDebug()<<"\nresource**year ="<< s_years.toString();
//	qDebug()<<"\nresource**month= "<<s_month.toString();
	
	int intmonth = s_month.toString().toInt();
//	if (intmonth.isEmpty() ) qDebug()<<" test month false";
	int intyear = s_years.toString().toInt();

	QDate s_date(intyear,intmonth,1);
//	qDebug() << "**look here" << s_date.month() << "**"<<s_date.year();
	
	history.setDate(s_date);
	//TODO need to set the date attributes in history instance
      }	else if ( reader.name() == QLatin1String( "contact" ) ) {
//	qDebug() <<" contact detected";
	
        if ( !seenHeader )
          return false;

        const QXmlStreamAttributes attributes = reader.attributes();
        const QStringRef contactId = attributes.value( QLatin1String( "contactId" ) );
	const QStringRef type = attributes.value( QLatin1String( "type" ) );
	
        if ( contactId.isEmpty() ) {
//	  qDebug() << "contact id empty";
          return false;
	}
	if ( type.toString() == QLatin1String( "myself" ) ) {
//	  qDebug()<<"myself detected local contact set"<<contactId.toString();
	  history.setLocalContactId( contactId.toString() );
	  seenlocalcontact=true;
	}
	else if (type.toString().isEmpty() && seenlocalcontact) {
//	  qDebug() << " remote contact id set" << contactId.toString();
	  history.setRemoteContactId( contactId.toString() );
	}
      } 
    } else if ( reader.isEndElement() ) {
      if ( reader.name() == QLatin1String( "head" ) )
        break;
    }
  }

  return seenHeader && !history.localContactId().isEmpty() &&
         !history.remoteContactId().isEmpty();
}

static bool readMessages( QXmlStreamReader &reader, History &history )
{
//  qDebug() << " read messages entered" ;
  bool seenMessages = false;
  
  while ( !reader.atEnd() ) {
    reader.readNext();

    if ( reader.isStartElement() ) {
      if ( reader.name() == QLatin1String( "msg" ) ) {
	seenMessages = true;
	
        const QXmlStreamAttributes attributes = reader.attributes();
	const QStringRef nick = attributes.value( QLatin1String( "nick" ) );
        const QStringRef in = attributes.value( QLatin1String( "in" ) );
	const QStringRef contactId  = attributes.value( QLatin1String( "from" ) );
        const QStringRef whenString = attributes.value( QLatin1String( "time" ) );
        if ( contactId.isEmpty() || whenString.isEmpty() )
          return false;
        const QDateTime timestamp =
            QDateTime::fromString( whenString.toString(), "d h:m:s" );
//	    qDebug() << "res**" << whenString.toString()<<"*"<<timestamp.toString();
//        if ( !timestamp.isValid() )
//          return false;
        QString text = reader.readElementText();
        History::Message message;
	message.setNick( nick.toString()  );
	message.setIn( in.toString() );
        message.setSender( contactId.toString() );
        message.setTimestamp( timestamp );
        message.setText( text );
        history.addMessage( message );
      }
    } else if ( reader.isEndElement() ) {
      if ( reader.name() == QLatin1String( "kopete-history" ) ) {
//	qDebug() << " kopete-history end element detected";
        break;
      }
    }
  }

  return seenMessages;
}

bool HistoryXmlIo::readHistoryFromXml( QIODevice *device, History &history )
{
  if ( device == 0 || !device->isReadable() )
    return false;

  history = History();

  QXmlStreamReader reader( device );

  while ( !reader.atEnd() ) {
    reader.readNext();

    if ( reader.isStartElement() ) {
      if ( reader.name() == QLatin1String( "kopete-history" ) ) {
//	qDebug() <<" kopetehistory detected" ;
        if ( !readHeader( reader, history ) ){
//	  qDebug() <<" read header returned false";
          history = History();
          return false;
        }

        if ( !readMessages( reader, history ) ) {
//	  qDebug() << " readmessages returned false";
          history = History();
          return false;
        }

        break;
      } else {
        return false;
      }
    }
  }

  return true;
}
bool HistoryXmlIo::writeHistoryHeaderToXml( const History &history, QIODevice *device )
{
  if ( device == 0 || !device->isWritable() )
    return false;
 
  QXmlStreamWriter writer( device );
  writer.setAutoFormatting( true );
 
  writer.writeStartDocument();
 
  writeHeader( history, writer );
 
  writer.writeEndDocument();
 
  return true;
}
 
bool HistoryXmlIo::readHistoryHeaderFromXml( QIODevice *device, History &history )
{
  if ( device == 0 || !device->isReadable() )
    return false;
 
  QXmlStreamReader reader( device );
 
  History partialHistory;
  if ( !readHeader( reader, partialHistory ) )
    return false;
 
  // only overwrite header fields
  history.setLocalContactId( partialHistory.localContactId() );
  history.setRemoteContactId( partialHistory.remoteContactId() );
 
  return true;
}
 
bool HistoryXmlIo::writeHistoryMessagesToXml( const History &history, QIODevice *device )
{
  if ( device == 0 || !device->isWritable() )
    return false;
 
  QXmlStreamWriter writer( device );
  writer.setAutoFormatting( true );
 
  writer.writeStartDocument();
 
  writeMessages( history, writer );
 
  writer.writeEndDocument();
 
  return true;
}
 
bool HistoryXmlIo::readHistoryMessagesFromXml( QIODevice *device, History &history )
{
  if ( device == 0 || !device->isReadable() )
    return false;
 
  QXmlStreamReader reader( device );
 
  History partialHistory;
  if ( !readMessages( reader, partialHistory ) )
    return false;
 
  // save header fields and then overwrite external instance
  partialHistory.setLocalContactId( history.localContactId() );
  partialHistory.setRemoteContactId( history.remoteContactId() );
 
  history = partialHistory;
 
  return true;
}
