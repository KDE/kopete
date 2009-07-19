/*    
    history.cpp

    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    Kopete    (c) 2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "history.h"
//#include "historyxmlio.h
#include <QDateTime>
#include <QList>
#include <QString>

//const char *HistoryXmlIo::HeaderPayload = "Header";
//const char *HistoryXmlIo::MessageListPayload = "MessageList";
const char *History::HeaderPayload = "Header";
const char *History::MessageListPayload = "MessageList";

class History::Message::Private : public QSharedData
{
public:
  Private() {}

  Private( const Private& other ) : QSharedData( other )
  {
    mSender = other.mSender;
    mText   = other.mText;
    mNick = other.mNick;
    mIn = other.mIn;
    mTimestamp = other.mTimestamp;
  }

public:
  QString mSender;
  QString mText;
  QString mNick;
  QString mIn;
  QDateTime mTimestamp;
};

History::Message::Message() : d( new Private )
{
}

History::Message::Message( const History::Message &other ) : d( other.d )
{
}

History::Message::~Message()
{
}

History::Message &History::Message::operator=( const History::Message &other )
{
  if ( this != &other )
    d = other.d;

  return *this;
}

void History::Message::setSender( const QString &contactId )
{
  d->mSender = contactId;
}

QString History::Message::sender() const
{
  return d->mSender;
}

void History::Message::setNick( const QString &nick )
{
  d->mNick = nick;
}

QString History::Message::nick() const
{
  return d->mNick;
}

void History::Message::setIn (const QString &in )
{
  d->mIn = in ;
}
QString History::Message::in() const
{
  return d->mIn;
}

void History::Message::setText( const QString &text )
{
  d->mText = text;
}

QString History::Message::text() const
{
  return d->mText;
}

void History::Message::setTimestamp( const QDateTime &timestamp )
{
  d->mTimestamp = timestamp;
}

QDateTime History::Message::timestamp() const
{
  return d->mTimestamp;
}

class History::Private : public QSharedData
{
public:
  Private() {}

  Private( const Private &other ) : QSharedData( other )
  {
    mLocal  = other.mLocal;
    mRemote = other.mRemote;
    mDate = other.mDate;
    
    mMessages = other.mMessages;
  }

public:
  QString mLocal;
  QString mRemote;
  QDate mDate;
  QList<History::Message> mMessages;
};

History::History() : d( new Private )
{
}

History::History( const History &other ) : d( other.d )
{
}

History::~History()
{
}

History &History::operator=( const History &other )
{
  if ( this != &other )
    d = other.d;

  return *this;
}

void History::setDate( const QDate &date )
{
  d->mDate = date ;
}

QDate History::date() const
{
  return d->mDate;
}

void History::setLocalContactId( const QString &contactId )
{
  d->mLocal = contactId;
}

QString History::localContactId() const
{
  return d->mLocal;
}

void History::setRemoteContactId( const QString &contactId )
{
  d->mRemote = contactId;
}

QString History::remoteContactId() const
{
  return d->mRemote;
}

void History::addMessage( const History::Message &message )
{
  d->mMessages << message;
}

History::Message::List History::messages() const
{
  return d->mMessages;
}

QString History::mimeType()
{
  return QLatin1String( "application/x-vnd.kde.kopetechathistory" );
}

