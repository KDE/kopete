/*
    Kopete Oscar Protocol
    oscartypeclasses.cpp - Oscar Type Definitions

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

#include "oscartypeclasses.h"
#include <QList>
#include <kdebug.h>
#include "oscarutils.h"
#include "buffer.h"


// using namespace Oscar;

Oscar::TLV::TLV()
:type(0), length(0)
{
}

Oscar::TLV::TLV( quint16 newType, quint16 newLength, char* newData )
:type( newType ), length( newLength ), data( QByteArray( newData, length ) )
{
}

Oscar::TLV::TLV( quint16 newType, quint16 newLength, const QByteArray& newData )
:type( newType ), length( newLength ), data( newData )
{
}

Oscar::TLV::TLV( const TLV& t )
:type( t.type ), length( t.length ), data( t.data )
{
}

Oscar::TLV::operator bool() const
{
	return type != 0;
}




//kate: indent-mode csands;
