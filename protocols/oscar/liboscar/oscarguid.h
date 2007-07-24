/*
    Kopete Oscar Protocol
    oscarguid.h - Oscar Guid Object

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef _OSCARGUID_H_
#define _OSCARGUID_H_

#include <QByteArray>
#include "liboscar_export.h"

namespace Oscar
{

/**
 * This class is just a simple little GUID, 16 bytes of data
 */

class LIBOSCAR_EXPORT Guid
{
public:
	Guid();
	Guid( const QByteArray & data );
	/** takes a string of hex values possibly separated by dashes */
	Guid( const QString & data );
	Guid( const Guid& other );

	/** get the data as a bytearray for decoding */
	const QByteArray data() const;

	/** set the data from a bytearray */
	void setData( const QByteArray& data );

	/** returns true if the guid is exactly 16 bytes */
	bool isValid() const;

	bool isEqual( const Guid &rhs, int n = 16 ) const;

	Guid &operator=( const Guid& rhs );
	bool operator==( const Guid& rhs ) const;
	operator QByteArray() const;

private:
	QByteArray m_data;
};

inline uint qHash(const Guid &key)
{
	return key.data().toUInt();
}

}

//kate: indent-mode csands; auto-insert-doxygen on; tab-width 4;

#endif
