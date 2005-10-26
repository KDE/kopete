/* -*- mode: C++; c-file-style: "gnu" -*- */
/* message.h: Qt wrapper for DBusMessage
 *
 * Copyright (C) 2003  Zack Rusin <zack@kde.org>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef DBUS_QT_MESSAGE_H
#define DBUS_QT_MESSAGE_H

#include <qvariant.h>
#include <qstring.h>
#include <qstringlist.h>

#include <dbus/dbus.h>

namespace DBusQt {

  class Message
  {
  public:
    class iterator {
    public:
      iterator();
      iterator( const iterator& );
      iterator( DBusMessage* msg );
      ~iterator();

      iterator& operator=( const iterator& );
      const QVariant& operator*() const;
      QVariant& operator*();
      iterator& operator++();
      iterator operator++(int);
      bool operator==( const iterator& it );
      bool operator!=( const iterator& it );

      QVariant var() const;
    protected:
      QVariant marshallBaseType( DBusMessageIter* i );
      void fillVar();
      struct IteratorData;
      IteratorData *d;
    };

    Message( int messageType );
    Message( DBusMessage * );//hide this one from the public implementation
    Message( const QString& service, const QString& path,
             const QString& interface, const QString& method );
    Message( const Message& replayingTo );
    Message( const QString& path, const QString& interface,
             const QString& name );
    Message( const Message& replayingTo, const QString& errorName,
             const QString& errorMessage );

    Message operator=( const Message& other );

    virtual ~Message();

    int type() const;

    void setPath( const QString& );
    QString path() const;

    void setInterface( const QString& );
    QString interface() const;

    void setMember( const QString& );
    QString member() const;

    void setErrorName( const QString& );
    QString errorName() const;

    void setDestination( const QString& );
    QString destination() const;

    bool    setSender( const QString& sender );
    QString    sender() const;
	
	void setAutoActivation(bool aa);
	bool autoActication();
	
	bool expectReply() const;

    QString signature() const;

    iterator begin() const;
    iterator end() const;

    QVariant at( int i );


  public:
    Message& operator<<( bool );
    Message& operator<<( Q_INT8 );
    Message& operator<<( Q_INT32 );
    Message& operator<<( Q_UINT32 );
    Message& operator<<( Q_INT64 );
    Message& operator<<( Q_UINT64 );
    Message& operator<<( double );
    Message& operator<<( const QString& );
    Message& operator<<( const QVariant& );
    //Message& operator<<();
    //Message& operator<<();
    //Message& operator<<();
    //Message& operator<<();
    //Message& operator<<();
    //Message& operator<<();
    //Message& operator<<();

  protected:
    friend class Connection;
    DBusMessage* message() const;

  private:
    struct Private;
    Private *d;
  };

}

#endif
