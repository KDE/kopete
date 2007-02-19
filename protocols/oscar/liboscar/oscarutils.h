/*
    Kopete Oscar Protocol
    oscarutils.h - Oscar Utility Functions

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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

#ifndef _OSCARUTILS_H_
#define _OSCARUTILS_H_

#include <qglobal.h>
#include <QList>
#include <qstring.h>
#include "oscartypes.h"
#include "buffer.h"
#include "contact.h"

namespace Oscar
{

///Normalize the contact name to all lowercase and no spaces
LIBOSCAR_EXPORT QString normalize( const QString& );

///compare TLVs for equality
LIBOSCAR_EXPORT bool operator==( TLV, TLV );

/**
 * Find the TLV corresponding to the type in the list
 */
LIBOSCAR_EXPORT TLV findTLV( const QList<TLV>&, int type );

/**
 * Update TLVs of SSI item from TLV list if necessary
 * \return true if something was updated
 */
LIBOSCAR_EXPORT bool updateTLVs( OContact& item, const QList<TLV>& list );

/**
 * Parse the character array for validness and a version string
 * \param buffer the buffer we'll be parsing for capabilities
 * \param versionString a QString reference that will contain the
 * version string of the detected client. Will be QString::null if 
 * no client is found
 * \return a DWORD containing a bit array of the capabilities we found
 */
Oscar::DWORD parseCapabilities( Buffer &inbuf, QString &versionString, int &xStatus );

/**
 * Get the name of the capability from its number
 */
const QString capName( int capNumber );

/**
 * Convert an IP address in dotted decimal notation to a 
 * numerical constant
 */
Oscar::DWORD getNumericalIP( const QString& address );

/**
 * Convert a numerical constant that is an IP address to
 * dotted decimal format
 */
QString getDottedDecimal( Oscar::DWORD address );

}

#endif

//kate: auto-insert-doxygen on; tab-width 4; indent-mode csands;
