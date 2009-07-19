/*
    history.h

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
#ifndef HISTORY_H
#define HISTORY_H

#include <QtCore/QSharedDataPointer>

class QDateTime;
class QString;
class QDate;

template <class T> class QList;


class History
{
public:
  class Message
  {
  public:
    typedef QList<Message> List;

    Message();
    Message( const Message &other );
    ~Message();

    Message &operator=( const Message &other );

    void setNick( const QString &nick );
    QString nick() const;
    
    void setIn ( const QString &in );
    QString in() const;
    
    void setSender( const QString &contactId );
    QString sender() const;

    void setText( const QString &text );
    QString text() const;

    void setTimestamp( const QDateTime &timestamp );
    QDateTime timestamp() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
  };

  History();
  History( const History &other );
  ~History();

  History &operator=( const History &other );

  void setDate( const QDate &date );
  QDate date() const ;
  
  void setLocalContactId( const QString &contactId );
  QString localContactId() const;

  void setRemoteContactId( const QString &contactId );
  QString remoteContactId() const;

  void addMessage( const Message &message );

  Message::List messages() const;

  static QString mimeType();

  static const char *HeaderPayload;
  static const char* MessageListPayload;

private:
  class Private;
  QSharedDataPointer<Private> d;
};

#endif