/*
    Kopete Oscar Protocol
    oscartypeclasses.h - Oscar Type Definitions

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef _OSCARTYPECLASSES_H_
#define _OSCARTYPECLASSES_H_

#include <qglobal.h>
#include <qstring.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <kopete_export.h>

namespace Oscar
{
class KOPETE_EXPORT TLV
{
public:
	
	TLV();
	TLV( Q_UINT16, Q_UINT16, char* data );
	TLV( Q_UINT16, Q_UINT16, const QByteArray& );
	TLV( const TLV& t );
	~TLV();
	
	operator bool() const;
	
	Q_UINT16 type;
	Q_UINT16 length;
	QByteArray data;

};

/**
 * This class is responsible for holding all the details
 * of a message and includes the following:
 * \li channel ( type )
 * \li encoding
 */

class KOPETE_EXPORT Message
{
public:
	
	enum {
		Normal = 0x0000,
		AutoResponse = 0x0001,
		WWP = 0x0002,
		EMail = 0x0004,
		UCS2 = 0x0010,
		Latin = 0x0020,
		UTF8 = 0x0040
	};
	
	Message();
	Message( const QString& messageText, int channel, int properties = 0, QDateTime timestamp = QDateTime() ); 
	Message( const Oscar::Message& m );
	
	/** Get the sender of the message */
	QString sender() const;
	
	/** Set the sender of the message */
	void setSender( const QString& sender );
	
	/** Get the receiver of the message */
	QString receiver() const;
	
	/** Set the receiver of the message */
	void setReceiver( const QString& receiver);
	
	/** get the message text */
	QString text() const;
	
	/** set the message text */
	void setText( const QString& newText );
	
	/** get the message properties */
	int properties() const;
	
	/** add a property to the message */
	void addProperty( int prop );
	
	/** get the channel ( type ) of the message */
	int type() const;
	
	/** set the channel ( type ) of the message */
	void setType( int newType );
	
	/** get the timestamp of the message */
	QDateTime timestamp() const;
	
	/** set the timestamp of the message */
	void setTimestamp(QDateTime ts);
	
	operator bool() const;
	
private:
	QString m_sender;
	QString m_receiver;
	int m_channel;
	int m_properties;
	QString m_text;
	QDateTime m_timestamp;
};

class KOPETE_EXPORT SSI
{
public:
	SSI();
	SSI( const QString &name, int gid, int bid, int type, const QValueList<TLV>& tlvlist, int tlvLength = 0 );
	SSI( const SSI& other );
	~SSI();
	
	/** Get the validity of this item */
	bool isValid() const;
	
	/** \brief The name of this SSI item.
	 * This is usually the screenname, ICQ number, or group name. */
	QString name() const;
	
	/** \brief The group id of the SSI item */
	Q_UINT16 gid() const;
	
	/** \brief The buddy id of the SSI item */
	Q_UINT16 bid() const;
	
	/**
	 * \brief The type of the SSI Item.
	 * see ROSTER_* defines on oscartypes.h
	 * Use a value of 0xFFFF for an SSI item not on the server list
	 */
	Q_UINT16 type() const;
	
	/** \brief the TLV list for the item */
	const QValueList<TLV>& tlvList() const;
	
	/** \brief Set the TLV list for the item */
	void setTLVList( QValueList<TLV> );
	
	/**
	 * \brief Set the length of the TLV list
	 *
	 * This is not the number of items in the list!! It's the aggregation of the
	 * sizes of the TLVs
	 */
	void setTLVListLength( Q_UINT16 newLength );
	
	/** \brief Get the TLV list length */
	Q_UINT16 tlvListLength() const;
	
	/**
	 * Get the alias for the SSI item
	 * This is TLV 0x0131 in an SSI item
	 */
	QString alias() const;
	
	/**
	 * Set the alias for the SSI item
	 * This should be done after a successful modification of the item
	 * on the server
	 */
	void setAlias( const QString& newAlias );
	
	/** \brief Indicates we're awaiting authorization from this item */
	bool waitingAuth() const;
	
	/** Set whether we are waiting authorization or not from this item */
	void setWaitingAuth( bool waiting );
	
	void setIconHash( QByteArray hash );
	
	QByteArray iconHash() const;
	
	/** \brief String representation of our SSI object */
	QString toString() const;
	
	bool operator==( const SSI& item ) const;
	operator bool() const;
	
private:
	void refreshTLVLength();
	
	//! parse the TLVs checking for aliases and auth and stuff like that
	void checkTLVs();
	
private:
	QString m_name;
	int m_gid;
	int m_bid;
	int m_type;
	QValueList<TLV> m_tlvList;
	int m_tlvLength;
	bool m_waitingAuth;
	QString m_alias;
	
};

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

#endif
