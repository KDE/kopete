/*
    gwfield.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qcstring.h>

#include "gwfield.h"

using namespace Field;

/* === FieldList ==================================================== */
FieldList::~FieldList()
{
	FieldListIterator it = begin();
	FieldListIterator theEnd = end();
	int index = 0;
	for ( ; it != theEnd; ++it, ++index )
		delete( *it );
}

FieldListIterator FieldList::find( QCString tag )
{
	FieldListIterator it = begin();
	return find( it, tag );
}

FieldListIterator FieldList::find( FieldListIterator it, QCString tag )
{
	FieldListIterator theEnd = end();
	for ( ; it != theEnd; ++it )
		if ( (*it)->tag() == tag )
			break;
	return it;
}

int FieldList::findIndex( QCString tag )
{
	FieldListIterator it = begin();
	FieldListIterator theEnd = end();
	int index = 0;
	for ( ; it != theEnd; ++it, ++index )
		if ( (*it)->tag() == tag )
			return index;
			
	return -1;
}

/* === FieldBase ========================================================= */

FieldBase::FieldBase( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type )
: m_tag( tag ), m_method( method ), m_flags( flags ), m_type( type )
{

}

QCString FieldBase::tag() const
{
	return m_tag;
}

Q_UINT8 FieldBase::method() const
{
	return m_method;
}

Q_UINT8 FieldBase::flags() const
{
	return m_flags;
}

Q_UINT8 FieldBase::type() const
{
	return m_type;
}

void FieldBase::setFlags( const Q_UINT8 flags )
{
	m_flags = flags;
}

/* === SingleField ========================================================= */

SingleField::SingleField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type, QVariant value )
: FieldBase( tag, method, flags, type ), m_value( value )
{
}

SingleField::SingleField( QCString tag, Q_UINT8 flags, Q_UINT8 type, QVariant value )
: FieldBase( tag, NMFIELD_METHOD_VALID, flags, type ), m_value( value )
{
}

SingleField::~SingleField()
{
}

void SingleField::setValue( const QVariant v )
{
	m_value = v;
}

QVariant SingleField::value() const
{
	return m_value;
}

/* === MultiField ========================================================= */

MultiField::MultiField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type, FieldList fields )
: FieldBase( tag, method, flags, type ), m_fields( fields )
{
}

MultiField::MultiField( QCString tag, Q_UINT8 method, Q_UINT8 flags, Q_UINT8 type )
: FieldBase( tag, method, flags, type )
{
}

FieldList MultiField::fields() const
{
	return m_fields;
}

void MultiField::setFields( FieldList fields )
{
	m_fields = fields;
}
