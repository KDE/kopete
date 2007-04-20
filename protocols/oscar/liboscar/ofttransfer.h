/*

    Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef OFTTRANSFER_H
#define OFTTRANSFER_H

#include "oscartypes.h"
#include "buffer.h"
#include "transfer.h"

using namespace Oscar;

class OftTransfer : public Transfer
{
public:

	explicit OftTransfer( OFT data, Buffer* buffer = 0 );
	OftTransfer();
	virtual ~OftTransfer();

	virtual TransferType type() const;
	virtual QByteArray toWire();

	//! Set the OFT data
	void setData( OFT data );

	//! Get the OFT data
	OFT data() const;

	//! Get the validity of the OFT header
	bool oftValid() const;
	
private:
	//! Converts fileName from Unicode to QByteArray;
	QByteArray encodeFileName( const QString &fileName, int &encodingType ) const;

	OFT m_data; //much easier to keep it all in a struct

	bool m_isOftValid;
	
};


#endif
