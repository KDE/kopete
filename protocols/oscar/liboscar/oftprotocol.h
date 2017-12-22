/*
    Kopete Oscar Protocol
    oftprotocol.h - reads the protocol used for oscar filetransfers

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Based on code copyright (c) 2004 SUSE Linux AG <http://www.suse.com>

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

#ifndef OSCAR_OFTPROTOCOL_H
#define OSCAR_OFTPROTOCOL_H

#include "inputprotocolbase.h"

class Transfer;


class OftProtocol : public InputProtocolBase
{
Q_OBJECT
public:
	OftProtocol( QObject *parent = nullptr );
	~OftProtocol();

	/**
	 * Attempt to parse the supplied data into an @ref OftTransfer object.
	 * The exact state of the parse attempt can be read using @ref state.
	 * @param rawData The unparsed data.
	 * @param bytes An integer used to return the number of bytes read.
	 * @return A pointer to an OftTransfer object if successful, otherwise 0.  The caller is responsible for deleting this object.
	 */
	Transfer * parse( const QByteArray &, uint & bytes ) Q_DECL_OVERRIDE;

};

#endif
