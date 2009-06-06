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
#include <QString>
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

/**
 * Searches all QTextCodec objects and returns the one which best matches name.
 * Returns 0 if no codec matching the name name could be found.
 */
QTextCodec * codecForName( const QByteArray& name );

}

#endif

//kate: auto-insert-doxygen on; tab-width 4; indent-mode csands;
