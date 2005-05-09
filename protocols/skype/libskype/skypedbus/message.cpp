
/* -*- mode: C++; c-file-style: "gnu" -*- */

/* message.cpp: Qt wrapper for DBusMessage
 *
 * Copyright (C) 2003  Zack Rusin <zack@kde.org>
 *
 * Licensed under the Academic Free License version 2.0
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

#define DBUS_API_SUBJECT_TO_CHANGE

#include "message.h"

#include <qmap.h>

#include <cstdlib>

namespace DBusQt
{

	struct Message::iterator::IteratorData
	{
		DBusMessageIter *iter;
		QVariant var;
		bool end;
		DBusMessage *mesg;
	};

/**
 * Iterator.
 */
	 Message::iterator::iterator(  )
	{
		d = new IteratorData;
		d->iter = 0;
		d->end = true;
	}

/**
 * Constructs iterator for the message.
 * @param msg message whose fields we want to iterate
 */
	Message::iterator::iterator( DBusMessage * msg )
	{
		d = new IteratorData;
		d->mesg = msg;
		d->iter =
			static_cast <
			DBusMessageIter * >( malloc( sizeof( DBusMessageIter ) ) );
		dbus_message_iter_init( d->mesg, d->iter );
		if ( !d->iter )
		{
			qDebug( "No iterator??" );
		}
		fillVar(  );
		d->end = false;
	}

/**
 * Copy constructor for the iterator.
 * @param itr iterator
 */
	Message::iterator::iterator( const iterator & itr )
	{
		d = new IteratorData;
		d->iter = itr.d->iter;
		d->var = itr.d->var;
		d->end = itr.d->end;
	}

/**
 * Destructor.
 */
	Message::iterator::~iterator(  )
	{
		free( d->iter );
		delete d;

		d = 0;
	}

/**
 * Creates an iterator equal to the @p itr iterator
 * @param itr other iterator
 * @return
 */
	Message::iterator & Message::iterator::operator=( const iterator & itr )
	{
		IteratorData *tmp = new IteratorData;

		tmp->iter = itr.d->iter;
		tmp->var = itr.d->var;
		tmp->end = itr.d->end;
		delete d;

		d = tmp;
		return *this;
	}

/**
 * Returns the constant QVariant held by the iterator.
 * @return the constant reference to QVariant held by this iterator
 */
	const QVariant & Message::iterator::operator*(  ) const
	{
		return d->var;
	}

/**
 * Returns the QVariant held by the iterator.
 * @return reference to QVariant held by this iterator
 */
	QVariant & Message::iterator::operator*(  )
	{
		return d->var;
	}

/**
 * Moves to the next field and return a reference to itself after
 * incrementing.
 * @return reference to self after incrementing
 */
	Message::iterator & Message::iterator::operator++(  )
	{
		if ( d->end )
			return *this;

		if ( dbus_message_iter_next( d->iter ) )
		{
			fillVar(  );
		}
		else
		{
			d->end = true;
			d->var = QVariant(  );
		}
		return *this;
	}

/**
 * Moves to the next field and returns self before incrementing.
 * @return self before incrementing
 */
	Message::iterator Message::iterator::operator++( int )
	{
		iterator itr( *this );

		operator++(  );
		return itr;
	}

/**
 * Compares this iterator to @p it iterator.
 * @param it the iterator to which we're comparing this one to
 * @return true if they're equal, false otherwise
 */
	bool Message::iterator::operator==( const iterator & it )
	{
		if ( d->end == it.d->end )
		{
			if ( d->end == true )
			{
				return true;
			}
			else
			{
				return d->var == it.d->var;
			}
		}
		else
			return false;
	}

/**
 * Compares two iterators.
 * @param it The other iterator.
 * @return true if two iterators are not equal, false
 *         otherwise
 */
	bool Message::iterator::operator!=( const iterator & it )
	{
		return !operator==( it );
	}

	QVariant Message::iterator::marshallBaseType( DBusMessageIter * i )
	{
		QVariant ret;

		switch ( dbus_message_iter_get_arg_type( i ) )
		{
		case DBUS_TYPE_INT32:
			ret = QVariant( dbus_message_iter_get_int32( i ) );
			break;
		case DBUS_TYPE_UINT32:
			ret = QVariant( dbus_message_iter_get_uint32( i ) );
			break;
		case DBUS_TYPE_DOUBLE:
			ret = QVariant( dbus_message_iter_get_double( i ) );
			break;
		case DBUS_TYPE_BOOLEAN:
			ret = QVariant( dbus_message_iter_get_boolean( i ) );
			break;
		case DBUS_TYPE_STRING:
		{
			char *str = dbus_message_iter_get_string( i );

			ret = QVariant( QString::fromUtf8( str ) );
			dbus_free( str );
		}
			break;
		default:
			ret = QVariant(  );
			break;
		}

		return ret;
	}

/**
 * Fills QVariant based on what current DBusMessageIter helds.
 */
	void Message::iterator::fillVar(  )
	{
		switch ( dbus_message_iter_get_arg_type( d->iter ) )
		{
		case DBUS_TYPE_INT32:
		case DBUS_TYPE_UINT32:
		case DBUS_TYPE_DOUBLE:
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_BOOLEAN:
			qDebug( "Message::iterator::fillVar, type == DBUS_TYPE_STRING" );
			d->var = marshallBaseType( d->iter );
			break;
		case DBUS_TYPE_ARRAY:
		{
			switch ( dbus_message_iter_get_array_type( d->iter ) )
			{
			case DBUS_TYPE_STRING:
			{
				QStringList tempList;
				int count;
				char **charArray;

				dbus_message_iter_get_string_array( d->iter,
													&charArray, &count );

				for ( int i = 0; i < count; i++ )
				{
					tempList.append( QString( charArray[i] ) );
				}

				d->var = QVariant( tempList );
				dbus_free( charArray );
				break;
			}
			default:
				qDebug( "Array of type not implemented" );
				d->var = QVariant(  );
				break;
			}
			break;
		}
		case DBUS_TYPE_DICT:
		{
			qDebug( "Got a hash!" );
			QMap < QString, QVariant > tempMap;
			DBusMessageIter dictIter;

			dbus_message_iter_init_dict_iterator( d->iter, &dictIter );
			do
			{
				char *key = dbus_message_iter_get_dict_key( &dictIter );

				tempMap[key] = marshallBaseType( &dictIter );
				dbus_free( key );
				dbus_message_iter_next( &dictIter );
			}
			while ( dbus_message_iter_has_next( &dictIter ) );
			d->var = QVariant( tempMap );
			break;
			qDebug( "Hash/Dict type not implemented" );
			d->var = QVariant(  );
			break;
		}
		default:
			qDebug( "not implemented" );
			d->var = QVariant(  );
			break;
		}
	}

/**
 * Returns a QVariant help by this iterator.
 * @return QVariant held by this iterator
 */
	QVariant Message::iterator::var(  ) const
	{
		return d->var;
	}

	struct Message::Private
	{
		DBusMessage *msg;
	};

	Message::Message( DBusMessage * m )
	{
		d = new Private;
		d->msg = m;
	}

/**
 *
 */
	Message::Message( int messageType )
	{
		d = new Private;
		d->msg = dbus_message_new( messageType );
	}

/**
 * Constructs a new Message with the given service and name.
 * @param service service service that the message should be sent to
 * @param name name of the message
 */
	Message::Message( const QString & service, const QString & path,
					  const QString & interface, const QString & method )
	{
		d = new Private;
		d->msg =
			dbus_message_new_method_call( service.latin1(  ), path.latin1(  ),
										  interface.latin1(  ),
										  method.latin1(  ) );
	}

/**
 * Constructs a message that is a reply to some other
 * message.
 * @param name the name of the message
 * @param replayingTo original_message the message which the created
 * message is a reply to.
 */
	Message::Message( const Message & replayingTo )
	{
		d = new Private;
		d->msg = dbus_message_new_method_return( replayingTo.d->msg );
	}

	Message::Message( const QString & path, const QString & interface,
					  const QString & name )
	{
		qDebug( "Message::Message" );
		d = new Private;
		d->msg = dbus_message_new_signal( path.ascii(  ), interface.ascii(  ),
										  name.ascii(  ) );
	}

	Message::Message( const Message & replayingTo, const QString & errorName,
					  const QString & errorMessage )
	{
		d = new Private;
		d->msg =
			dbus_message_new_error( replayingTo.d->msg, errorName.utf8(  ),
									errorMessage.utf8(  ) );
	}

	Message Message::operator=( const Message & other )
	{
		//FIXME: ref the other.d->msg instead of copying it?
	}

/**
 * Destructs message.
 */
	Message::~Message(  )
	{
		if ( d->msg )
		{
			dbus_message_unref( d->msg );
		}
		delete d;

		d = 0;
	}

	int Message::type(  ) const
	{
		return dbus_message_get_type( d->msg );
	}

	void Message::setAutoActivation( bool aa )
	{
		dbus_message_set_auto_activation( d->msg, aa );
	}

	bool Message::autoActication(  )
	{
		return dbus_message_get_auto_activation( d->msg );
	}

	void Message::setPath( const QString & path )
	{
		dbus_message_set_path( d->msg, path.ascii(  ) );
	}

	QString Message::path(  ) const
	{
		return dbus_message_get_path( d->msg );
	}

	void Message::setInterface( const QString & iface )
	{
		dbus_message_set_interface( d->msg, iface.ascii(  ) );
	}

	QString Message::interface(  ) const
	{
		return dbus_message_get_interface( d->msg );
	}

	void Message::setMember( const QString & member )
	{
		dbus_message_set_member( d->msg, member.ascii(  ) );
	}

	QString Message::member(  ) const
	{
		return dbus_message_get_member( d->msg );
	}

	void Message::setErrorName( const QString & err )
	{
		dbus_message_set_error_name( d->msg, err.utf8() );
	}

	QString Message::errorName(  ) const
	{
		return dbus_message_get_error_name( d->msg );
	}

	void Message::setDestination( const QString & dest )
	{
		dbus_message_set_destination( d->msg, dest.utf8() );
	}

	QString Message::destination(  ) const
	{
		return dbus_message_get_destination( d->msg );
	}

/**
 * Sets the message sender.
 * @param sender the sender
 * @return false if unsuccessful
 */
	bool Message::setSender( const QString & sender )
	{
		return dbus_message_set_sender( d->msg, sender.latin1(  ) );
	}

/**
 * Returns sender of this message.
 * @return sender
 */
	QString Message::sender(  ) const
	{
		return dbus_message_get_sender( d->msg );
	}

	bool Message::expectReply(  ) const
	{
		return !dbus_message_get_no_reply( d->msg );
	}

	QString Message::signature(  ) const
	{
		return dbus_message_get_signature( d->msg );
	}

/**
 * Returns the starting iterator for the fields of this
 * message.
 * @return starting iterator
 */
	Message::iterator Message::begin(  ) const
	{
		return iterator( d->msg );
	}

/**
 * Returns the ending iterator for the fields of this
 * message.
 * @return ending iterator
 */
	Message::iterator Message::end(  ) const
	{
		return iterator(  );
	}

/**
 * Returns the field at position @p i
 * @param i position of the wanted field
 * @return QVariant at position @p i or an empty QVariant
 */
	QVariant Message::at( int i )
	{
		iterator itr( d->msg );

		while ( i-- )
		{
			if ( itr == end(  ) )
				return QVariant(  );	//nothing there
			++itr;
		}
		return *itr;
	}

/**
 * The underlying DBusMessage of this class.
 * @return DBusMessage pointer.
 */
	DBusMessage *Message::message(  ) const
	{
		return d->msg;
	}

	Message & Message::operator<<( bool b )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_BOOLEAN, b,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( Q_INT8 byte )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_BYTE, byte,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( Q_INT32 num )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_INT32, num,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( Q_UINT32 num )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_UINT32, num,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( Q_INT64 num )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_INT64, num,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( Q_UINT64 num )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_UINT64, num,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( double num )
	{
		dbus_message_append_args( d->msg, DBUS_TYPE_DOUBLE, num,
								  DBUS_TYPE_INVALID );
	}

	Message & Message::operator<<( const QString & str )
	{
		dbus_message_append_args( d->msg,
								  DBUS_TYPE_STRING,
								  ( const char * ) ( str.utf8(  ) ), 0 );
	}

	Message & Message::operator<<( const QVariant & custom )
	{
		//FIXME: imeplement
	}

}

