/*
    Kopete Oscar Protocol
    OContact Object Definition

    Copyright (c) 2006 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIBOSCAR_CONTACT_H
#define LIBOSCAR_CONTACT_H

#include "oscartypeclasses.h"
#include "oscartypes.h"

class LIBOSCAR_EXPORT OContact
{
public:
	OContact();
	OContact( const QString &name, int gid, int bid, int type, const QList<Oscar::TLV>& tlvlist, int tlvLength = 0 );
	OContact( const OContact& other );

	/** 
	 * Get the validity of this item
	 * Invalid items have a type of 0xFFFF
	 */
	bool isValid() const;

	/**
	 * \brief The name of this OContact item.
	 * This is usually the screenname, ICQ number, or group name.
	 */
	QString name() const;

	/**
	 * \brief The group id of the OContact item
	 * The group id is given to us by the server
	 */
	quint16 gid() const;

	/**
	 * \brief The buddy id of the OContact item
	 * The buddy id is given to us by the server
	 */
	quint16 bid() const;

	/**
	 * Check to see if the contact supports a certain capability 
	 * 
	 * Capabilities are defined in oscartypes.h
	 */
	bool supportsFeature( Oscar::Capability ) const;

	/**
	 * \brief The type of the OContact Item.
	 * The ROSTER defines in oscartypes.h detail what types we know about
	 * Use a value of 0xFFFF for an OContact item not on the server list
	 */
	quint16 type() const;

	/** \brief the TLV list for the item */
	const QList<Oscar::TLV>& tlvList() const;

	/** \brief Set the TLV list for the item */
	void setTLVList( QList<Oscar::TLV> );

	/**
	 * \brief Set the length of the TLV list
	 *
	 * This is not the number of items in the list!! It's the aggregation of the
	 * sizes of the TLVs
	 */
	void setTLVListLength( quint16 newLength );

	/** \brief Get the TLV list length */
	quint16 tlvListLength() const;

	/**
	 * Get the alias for the OContact item
	 * This is TLV 0x0131 in an OContact item
	 */
	QString alias() const;

	/**
	 * Set the alias for the OContact item
	 * This should be done after a successful modification of the item
	 * on the server
	 */
	void setAlias( const QString& newAlias );

	/** \brief Indicates we're awaiting authorization from this item */
	bool waitingAuth() const;

	/** Set whether we are waiting authorization or not from this item */
	void setWaitingAuth( bool waiting );

	//! Set the icon hash for this contact.
	void setIconHash( QByteArray hash );

	//! Get the icon hash for this contact.
	QByteArray iconHash() const;

	//! Set the meta info id for this contact.
	void setMetaInfoId( const QByteArray& id );

	//! Get the meta info id for this contact.
	QByteArray metaInfoId() const;

	/** \brief String representation of our OContact object */
	QString toString() const;

	bool operator==( const OContact& item ) const;
	operator bool() const;

	operator QByteArray() const;

	void refreshTLVLength();

	//! parse the TLVs checking for aliases and auth and stuff like that
	void checkTLVs();

private:
	QString m_name;
	int m_gid;
	int m_bid;
	int m_type;
	QList<Oscar::TLV> m_tlvList;
	int m_tlvLength;
	bool m_waitingAuth;
	QString m_alias;
	QByteArray m_hash;
	QByteArray m_metaInfoId;
	Oscar::Capabilities m_caps;
};

#endif
//kate: space-indent off; tab-width 4; auto-insert-doxygen on; indent-mode csands;
