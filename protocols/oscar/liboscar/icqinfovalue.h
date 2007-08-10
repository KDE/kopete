/*
Kopete Oscar Protocol
icqinfovalue.h - ICQ Info Value Template

Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

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

#ifndef ICQINFOVALUE_H
#define ICQINFOVALUE_H

template <class T> class ICQInfoValue
{
public:
	/**
	 * ICQInfoValue constructor
	 * @param assumeDirty if true hasChanged() will be false only when operator= was used before,
	 * if loadStore is false hasChanged() will be true only if set was used.
	 */
	ICQInfoValue( bool assumeDirty = true );

	void set( const T & value );
	const T &get() const;

	T & operator=( const T & value );
	void init( const T & value );

	bool hasChanged() const;

private:
	T m_value;
	bool m_dirty;
	bool m_assumeDirty;
};

template <class T>
ICQInfoValue<T>::ICQInfoValue( bool assumeDirty )
{
	m_assumeDirty = assumeDirty;
	m_dirty = assumeDirty;
}

template <class T>
void ICQInfoValue<T>::set( const T & value )
{
	if ( m_value != value || !m_assumeDirty )
	{
		m_value = value;
		m_dirty = true;
	}
}

template <class T>
const T &ICQInfoValue<T>::get() const
{
	return m_value;
}

template <class T>
T & ICQInfoValue<T>::operator=( const T & value )
{
	m_value = value;
	m_dirty = false;
	return m_value;
}

template <class T>
void ICQInfoValue<T>::init( const T & value )
{
	m_value = value;
}

template <class T>
bool ICQInfoValue<T>::hasChanged() const
{
	return m_dirty;
}

#endif
