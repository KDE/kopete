/*
    gwfield.cpp - Fields used for Request/Response data in GroupWise
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qcstring.h>

#include "gwerror.h"

#ifdef LIBGW_USE_KDEBUG
  #include <kdebug.h>
#endif

#include "gwfield.h"
#include <iostream>

using namespace Field;
using namespace std;

/* === FieldList ==================================================== */
FieldList::~FieldList()
{
}

FieldListIterator FieldList::find( QCString tag )
{
	FieldListIterator it = begin();
	return find( it, tag );
}

FieldListIterator FieldList::find( FieldListIterator &it, QCString tag )
{
	FieldListIterator theEnd = end();
	//cout << "FieldList::find() looking for " << tag.data() << endl;
	for ( ; it != theEnd; ++it )
	{
		//cout << " - on " << (*it)->tag().data() << endl;
		if ( (*it)->tag() == tag )
			break;
	}
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

void FieldList::dump( bool recursive, int offset )
{
	const FieldListIterator myEnd = end();
	if ( !offset )
		kdDebug( GROUPWISE_DEBUG_LIBGW ) << k_funcinfo << ( recursive ? ", recursively" : ", non-recursive" ) << endl;
	for( FieldListIterator it = begin(); it != myEnd; ++it )
	{
		QString s;
		s.fill(' ', offset*2 );
		s.append( (*it)->tag() );
		SingleField * sf;
		if ( ( sf = dynamic_cast<SingleField*>( *it ) ) )
		{
			s.append( " :" );
			s.append( sf->value().toString() );
		}
		kdDebug( GROUPWISE_DEBUG_LIBGW ) << s << endl;
		if ( recursive )
		{
			MultiField * mf;
			if ( ( mf = dynamic_cast<MultiField*>( *it ) ) )
				mf->fields().dump( recursive, offset+1 );
		}
	}
}

void FieldList::purge()
{
	Field::FieldListIterator it = begin();
	Field::FieldListIterator theEnd = end();
	int index = 0;
	for ( ; it != theEnd; ++it, ++index )
		delete *it;
}

// THIS IS AN ATTEMPT TO HIDE THE POLYMORPHISM INSIDE THE LIST
// HOWEVER IT FAILS BECAUSE WE NEED BOTH THE ITERATOR AND THE CASTED Single|MultiField it points to

SingleField * FieldList::findSingleField( QCString tag )
{
	FieldListIterator it = begin();
	return findSingleField( it, tag );
}

SingleField * FieldList::findSingleField( FieldListIterator &it, QCString tag )
{
	FieldListIterator found = find( it, tag );
	if ( found == end() )
		return 0;
	else
		return dynamic_cast<SingleField *>( *found );
}

MultiField * FieldList::findMultiField( QCString tag )
{
	FieldListIterator it = begin();
	return findMultiField( it, tag );
}

MultiField * FieldList::findMultiField( FieldListIterator &it, QCString tag )
{
	FieldListIterator found = find( it, tag );
	if ( found == end() )
		return 0;
	else
	return dynamic_cast<MultiField *>( *found );
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

MultiField::~MultiField()
{
	m_fields.purge();
}

FieldList MultiField::fields() const
{
	return m_fields;
}

void MultiField::setFields( FieldList fields )
{
	m_fields = fields;
}
