/***************************************************************************
                          kopeteprotocol.cpp  -  description
                             -------------------
    begin                : Tue Jan 1 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopeteprotocol.h"

KopeteProtocol::KopeteProtocol()
{
}

KopeteProtocol::~KopeteProtocol()
{
}

QString KopeteProtocol::icon() const
{
	return m_icon;
}

void KopeteProtocol::setIcon( const QString &icon )
{
	m_icon = icon;
}

// vim: set noet ts=4 sts=4 sw=4:

