/*
    transfer.h - Kopete Groupwise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

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

#ifndef TRANSFER_H
#define TRANSFER_H

#include "oscartypes.h"
#include "buffer.h"


using namespace Oscar;

class Transfer
{
public:
        enum TransferType { RawTransfer, FlapTransfer, SnacTransfer, DIMTransfer, FileTransfer };
        Transfer();
	Transfer( Buffer* buf );
        virtual ~Transfer();

        virtual TransferType type() const;
	
	virtual QByteArray toWire();

	//! Set the data buffer
	void setBuffer( Buffer* buffer );

	//! Get the data buffer
	Buffer* buffer();
	
	const Buffer* buffer() const; //used for const transfer objects
	
	//! Get the validity of the data after the flap header
	bool dataValid() const;
	
	QString toString() const;
	
	void populateWireBuffer( int offset, const QByteArray& buffer );

protected:
	//! The wire-format representation of our buffer
	QByteArray m_wireFormat;

	//! The high-level representation of our data
	Buffer* m_buffer;

private:

	//! Flag to indicate whether we're a valid transfer
	bool m_isBufferValid;
	
};

class FlapTransfer : public Transfer
{
public:

	explicit FlapTransfer( Buffer* buffer, Oscar::BYTE chan = 0, Oscar::WORD seq = 0, Oscar::WORD len = 0 );
	FlapTransfer( FLAP f, Buffer* buffer );
	FlapTransfer();
	virtual ~FlapTransfer();

	virtual TransferType type() const;
	virtual QByteArray toWire();


	//! Set the FLAP channel
	void setFlapChannel( Oscar::BYTE channel );

	//! Get the FLAP channel
	Oscar::BYTE flapChannel() const;

	//! Set the FLAP sequence
	void setFlapSequence( Oscar::WORD seq );

	//! Get the FLAP sequence
	Oscar::WORD flapSequence() const;

	//! Set the length of the data after the FLAP
	void setFlapLength( Oscar::WORD len );

	//! Get the length of the data after the FLAP
	Oscar::WORD flapLength() const;

	//! Get the validity of the FLAP header
	bool flapValid() const;
	
private:
	Oscar::BYTE m_flapChannel;
	Oscar::WORD m_flapSequence;
	Oscar::WORD m_flapLength;

	bool m_isFlapValid;
	
};

/**
@author Matt Rogers
*/
class SnacTransfer : public FlapTransfer
{
public:

	/*SnacTransfer();*/
	explicit SnacTransfer( Buffer*, Oscar::BYTE chan = 0, Oscar::WORD seq = 0, Oscar::WORD len = 0, Oscar::WORD service = 0,
		 Oscar::WORD subtype = 0, Oscar::WORD flags = 0, Oscar::DWORD reqId = 0 );
	SnacTransfer( struct FLAP f, struct SNAC s, Buffer* buffer );
	SnacTransfer();
	virtual ~SnacTransfer();

	TransferType type() const;
	virtual QByteArray toWire();


	//! Set the SNAC service
	void setSnacService( Oscar::WORD service );

	//! Get the SNAC service
	Oscar::WORD snacService() const;

	//! Set the SNAC subtype
	void setSnacSubtype( Oscar::WORD subtype );

	//! Get the SNAC subtype
	Oscar::WORD snacSubtype() const;

	//! Set the SNAC flags
	void setSnacFlags( Oscar::WORD flags );

	//! Get the SNAC flags
	Oscar::WORD snacFlags() const;

	//! Set the SNAC request id
	void setSnacRequest( Oscar::DWORD id );

	//! Get the SNAC request id
	Oscar::DWORD snacRequest() const;

	//! Get the validity of the SNAC header
	bool snacValid() const;
	
	//! Get the SNAC header
	SNAC snac() const;

private:

	Oscar::WORD m_snacService;
	Oscar::WORD m_snacSubtype;
	Oscar::WORD m_snacFlags;
	Oscar::DWORD m_snacReqId;
	
	bool m_isSnacValid;
};

#endif
