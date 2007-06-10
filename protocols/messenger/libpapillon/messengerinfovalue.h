/*
Kopete Messenger Protocol
messengerinfovalue.h - Messenger Info Value Template

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

#ifndef MESSENGERINFOVALUE_H
#define MESSENGERINFOVALUE_H

template <class T> class MessengerInfoValue
{
public:
	MessengerInfoValue();

	void set( const T & value );
	const T &get() const;

	T & operator=( const T & value );
	void init( const T & value );

	bool hasChanged() const;

private:
	T m_value;
	bool m_dirty;
};

template <class T>
MessengerInfoValue<T>::MessengerInfoValue()
{
	m_dirty = true;
}

template <class T>
void MessengerInfoValue<T>::set( const T & value )
{
	if ( m_value != value )
	{
		m_value = value;
		m_dirty = true;
	}
}

template <class T>
const T &MessengerInfoValue<T>::get() const
{
	return m_value;
}

template <class T>
T & MessengerInfoValue<T>::operator=( const T & value )
{
	m_value = value;
	m_dirty = false;
	return m_value;
}

template <class T>
void MessengerInfoValue<T>::init( const T & value )
{
	m_value = value;
}

template <class T>
bool MessengerInfoValue<T>::hasChanged() const
{
	return m_dirty;
}

#endif
