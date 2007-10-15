/*
    Kopete Yahoo Protocol

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>
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

#ifndef YAHOO_YMSGPROTOCOL_H
#define YAHOO_YMSGPROTOCOL_H

#include "inputprotocolbase.h"


class YMSGProtocol : public InputProtocolBase
{
Q_OBJECT
public:
	

	YMSGProtocol( QObject *parent = 0 );
	~YMSGProtocol();
	
	/** 
	 * Attempt to parse the supplied data into an @ref YMSGTransfer object.  
	 * The exact state of the parse attempt can be read using @ref state. 
	 * @param rawData The unparsed data.
	 * @param bytes An integer used to return the number of bytes read.
	 * @return A pointer to an EventTransfer object if successful, otherwise 0.  The caller is responsible for deleting this object.
	 */
	Transfer * parse( const QByteArray &, uint & bytes );
};

#endif
